#include "graph.hpp"

#include <algorithm>

Node::Node(int assignedId, int assignedLevel) {
  id = assignedId;
  level = assignedLevel;
  neighbours.resize(level + 1);
}

void HNSW::normaliseVector(float *vecStart, int DIM) {
  float sqr_sum = 0.0;
  for (int i = 0; i < DIM; i++) {
    sqr_sum += ((vecStart[i]) * (vecStart[i]));
  }
  float mag = std::sqrt(sqr_sum);
  if (mag > 0) {
    for (auto i = 0; i < DIM; i++) {
      vecStart[i] /= mag;
    }
  }
}

HNSW::HNSW(int m, int efC, int efS, int dim) {
  DIM = dim;
  M = m;
  efConstruction = efC;
  efSearch = efS;
  mL = 1 / log(M);
  maxLevel = -1;
  gen = std::mt19937(std::random_device{}());
  dist = std::uniform_real_distribution<float>(std::numeric_limits<float>::epsilon(), 1);
  search_version = 0;
}

const float *HNSW::get_vector(int id) { return &flat_storage[id * DIM]; }

float *HNSW::get_vector_mutable(int id) { return &flat_storage[id * DIM]; }

float HNSW::distance(const float *vec1, const float *vec2) {
  float dot_prod = 0.0f;
  for (size_t i = 0; i < DIM; ++i) {
    dot_prod += vec1[i] * vec2[i];
  }
  return 1.0f - dot_prod;
}

int HNSW::assignLevel() {
  int level = floor(-log(dist(gen)) * mL);
  return level;
}

int HNSW::greedySearch(int entryNode, const float *vecStart, int level) {
  int current = entryNode;

  const float *entryStart = get_vector(current);
  float currentDist = distance(entryStart, vecStart);

  while (true) {
    bool changed = false;

    std::vector<int> &neighbours = nodes[current].neighbours[level];

    for (int i : neighbours) {
      float neighbourDist = distance(get_vector(i), vecStart);
      if (neighbourDist < currentDist) {
        current = i;
        currentDist = neighbourDist;
        changed = true;
      }
    }

    if (!changed) break;
  }
  return current;
}

void HNSW::insert(const std::vector<float> &vec) {
  int id = nodes.size();
  int level = assignLevel();

  flat_storage.insert(flat_storage.end(), vec.begin(), vec.end());

  float *vecStart = get_vector_mutable(id);

  normaliseVector(vecStart, DIM);

  Node newNode = Node(id, level);

  if (nodes.empty()) {
    nodes.push_back(newNode);
    entryPoint = id;
    maxLevel = level;
    visited.push_back(0);
    return;
  }

  nodes.push_back(newNode);
  visited.push_back(0);

  int current = entryPoint;
  for (int layer = maxLevel; layer >= level + 1; layer--) {
    current = greedySearch(current, get_vector(id), layer);
  }

  int startLayer = std::min(level, maxLevel);

  for (int layer = startLayer; layer >= 0; layer--) {
    if (current == -1) break;

    auto topCandidates = searchLayer(get_vector(id), current, layer, efConstruction);

    int closest_node_for_next_layer = -1;
    std::vector<int> candidateList;

    while (!topCandidates.empty()) {
      closest_node_for_next_layer = topCandidates.top().second;
      candidateList.push_back(closest_node_for_next_layer);
      topCandidates.pop();
    }

    // Assign all efConstruction candidates to neighbour list
    nodes[id].neighbours[layer] = candidateList;

    // Prune hndles the heuristic pruning of the farthest edges
    // (The lion doesn't concern itself with selecting the best edges)
    pruneNeighbours(id, layer);

    for (int survivingId : nodes[id].neighbours[layer]) {
      // Now we prune edges only the final surviving edges after pruning
      nodes[survivingId].neighbours[layer].push_back(id);
      pruneNeighbours(survivingId, layer);
    }

    if (nodes[id].neighbours[layer].empty()) break;

    current = closest_node_for_next_layer;
  }

  if (level > maxLevel) {
    entryPoint = id;
    maxLevel = level;
  }
}

std::priority_queue<std::pair<float, int>> HNSW::searchLayer(const float *query, int entryNode,
                                                             int layer, int ef) {
  /*
    Makes use of vectors linear nature to reduce memory allocation and
    reallocation, also reduces access time to O(1). Increment search version on
    each function call so the corresponding visited edges correspond to the
    version.
  */
  search_version++;
  if (search_version == 0) {
    std::fill(visited.begin(), visited.end(), 0);
    search_version = 1;
  }

  std::priority_queue<std::pair<float, int>, std::vector<std::pair<float, int>>,
                      std::greater<std::pair<float, int>>>
      candidateQueue;
  std::priority_queue<std::pair<float, int>, std::vector<std::pair<float, int>>> topCandidates;

  float entryNodeDist = distance(query, get_vector(entryNode));

  candidateQueue.push({entryNodeDist, entryNode});
  topCandidates.push({entryNodeDist, entryNode});
  visited[entryNode] = search_version;

  while (!candidateQueue.empty()) {
    auto current = candidateQueue.top();
    candidateQueue.pop();

    float currentDistance = current.first;
    int currentId = current.second;

    if (currentDistance > topCandidates.top().first) {
      break;
    }

    for (int neighbour : nodes[currentId].neighbours[layer]) {
      if (visited[neighbour] != search_version) {
        visited[neighbour] = search_version;
        float distNeighbour = distance(query, get_vector(neighbour));

        if (topCandidates.size() < (size_t)ef || distNeighbour < topCandidates.top().first) {
          candidateQueue.push({distNeighbour, neighbour});
          topCandidates.push({distNeighbour, neighbour});

          if (topCandidates.size() > (size_t)ef) {
            topCandidates.pop();
          }
        }
      }
    }
  }
  return topCandidates;
}

void HNSW::pruneNeighbours(int nodeId, int layer) {
  std::vector<int> &neighbours = nodes[nodeId].neighbours[layer];

  // Layer 0 gets double capacity to prevent dead ends at the base
  int maxM = (layer == 0) ? M * 2 : M;

  // Check if neighbours already withing M limit
  if (neighbours.size() <= (size_t)maxM) return;

  // Cache distances and sort closest-first
  std::vector<std::pair<float, int>> candidates;
  candidates.reserve(neighbours.size());
  for (int neighbour : neighbours) {
    float d = distance(get_vector(nodeId), get_vector(neighbour));
    candidates.push_back({d, neighbour});
  }
  std::sort(candidates.begin(), candidates.end());

  std::vector<int> keptNeighbours;
  keptNeighbours.reserve(maxM);
  std::vector<int> discardedNeighbours;
  discardedNeighbours.reserve(maxM);

  // The Heuristic Check
  for (auto &candidate : candidates) {
    if (keptNeighbours.size() >= (size_t)maxM) break;

    int candidateId = candidate.second;
    float distToNode = candidate.first;
    bool goodEdge = true;

    // Check if this candidate is closer to ANY already kept neighbour than to
    // the base node
    for (int keptId : keptNeighbours) {
      float distToKept = distance(get_vector(candidateId), get_vector(keptId));
      if (distToKept < distToNode) {
        goodEdge = false;
        break;
      }
    }

    if (goodEdge)
      keptNeighbours.push_back(candidateId);
    else
      discardedNeighbours.push_back(candidateId);
  }

  // Fallback: fill remaining slots with closest discarded nodes
  for (int discardedId : discardedNeighbours) {
    if (keptNeighbours.size() >= (size_t)maxM) break;
    keptNeighbours.push_back(discardedId);
  }

  neighbours = std::move(keptNeighbours);
}

std::vector<std::pair<int, float>> HNSW::search(std::vector<float> &query, int k) {
  // Normalize the query vector since all the added vectors are normalized
  float sqr_sum = 0.0f;
  for (float i : query) {
    sqr_sum += (i * i);
  }
  float mag = std::sqrt(sqr_sum);
  if (mag > 0) {
    for (float &val : query) {
      val /= mag;
    }
  }

  std::vector<std::pair<int, float>> topK;

  if (nodes.empty()) {
    return topK;
  }

  const float *queryStart = &query[0];
  int current = entryPoint;
  for (int i = maxLevel; i > 0; i--) {
    current = greedySearch(current, queryStart, i);
  }

  auto topCandidates = searchLayer(queryStart, current, 0, efSearch);
  while (topCandidates.size() > (size_t)k) {
    topCandidates.pop();
  }
  while (!topCandidates.empty()) {
    topK.push_back({topCandidates.top().second, topCandidates.top().first});
    topCandidates.pop();
  }

  std::reverse(topK.begin(), topK.end());
  return topK;
}
