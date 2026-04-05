#include "graph.hpp"

#include <algorithm>


Node::Node(int assignedId, std::vector<float> vecData, int assignedLevel) {
  id = assignedId;
  data = vecData;
  level = assignedLevel;
  neighbours.resize(level + 1);
  normaliseVector(data);
}

void Node::normaliseVector(std::vector<float> &vec) {
  /*
  FUNCTION:
    Normalizes the vector to reduce distance computation. Triggered on Node
  creation.

  FUNCTION PARAMETERS:
    vec: vector (float), The vector who is to be normalised
  */

  float sqr_sum = 0.0;
  for (float i : vec) {
    sqr_sum += (i * i);
  }
  float mag = std::sqrt(sqr_sum);
  if (mag > 0) {
    for (float &val : vec) {
      val /= mag;
    }
  }
}

HNSW::HNSW(int m, int efC, int efS) {
  M = m;
  efConstruction =
      efC;  // Max size of the candidate list during graph construction
  efSearch = efS;
  mL = 1 / log(M);
  maxLevel = -1;
  gen = std::mt19937(std::random_device{}());
  dist = std::uniform_real_distribution<float>(
      std::numeric_limits<float>::epsilon(), 1);
  search_version = 0;
}

float HNSW::distance(const std::vector<float> &vec1,
                     const std::vector<float> &vec2) {
  /*
  FUNCTION:
    Calculates the L2 distance between the two vectors.

  FUNCTION PARAMETERS:
    vec1, vec2: vector (float), The two vectors whose L2 distance is to be
  calculated
  */

  float dot_prod = 0;
  for (size_t i = 0; i < vec1.size(); ++i) {
    dot_prod += vec1[i] * vec2[i];
  }
  return 1.0f - dot_prod;
}

int HNSW::assignLevel() {
  /*
  FUNCTION:
    Assigns a random level to a new node using a random, exponentially decaying
  function

  FUNCTION PARAMETERS:

  */

  int level = floor(-log(dist(gen)) * mL);
  return level;
}

int HNSW::greedySearch(int entryNode, const std::vector<float> &queryVector,
                       int level) {
  /*
  FUNCTION:
    Greedy searches for the closest nodes starting from the entry node.

  FUNCTION PARAMETERS:
    entryNode: integer, The entry node from which the search begins for the
  layer. queryVector: vector (float), The vector for which we need to find the
  closest node level: integer, The level of the graph on which we are searching.
  */

  // Set the entry node as the current node
  // Better to reference using ids instead of passing Node objects

  int current = entryNode;

  float currentDist = distance(nodes[current].data, queryVector);

  while (true) {
    bool changed = false;

    std::vector<int> &neighbours = nodes[current].neighbours[level];

    for (int i : neighbours) {
      float neighbourDist = distance(nodes[i].data, queryVector);
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
  /*
  FUNCTION:
    Inserts a new node into the graph at the level assigned to it and connects
  it to other nodes.

  FUNCTION PARAMETERS:
    vec: vector (float), The vector data that is to be inserted

  */

  int id = nodes.size();
  int level = assignLevel();

  Node newNode = Node(id, vec, level);
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
    current = greedySearch(current, nodes[id].data, layer);
  }

  int startLayer = std::min(level, maxLevel);

  for (int layer = startLayer; layer >= 0; layer--) {
    if (current == -1) break;

    auto topCandidates =
        searchLayer(nodes[id].data, current, layer, efConstruction);

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

std::priority_queue<std::pair<float, int>> HNSW::searchLayer(
    const std::vector<float> &query, int entryNode, int layer, int ef) {
  /*
  FUNCTION:
    Searches the layer for the best possible ef number of candidates closest to
  query. Starts with neighbours of entryNode.

  FUNCTION PARAMETERS:
    query: vector (float), A reference to the query vector for which we need to
  find the closest neighbours entryNode: integer, The entry node to the layer
  where we begin our search layer: integer, The current layer of the graph we
  are searching on ef: integer, The hyperparameter efConstruction that decides
  the size of the candidate list
  */

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
  std::priority_queue<std::pair<float, int>, std::vector<std::pair<float, int>>>
      topCandidates;

  float entryNodeDist = distance(query, nodes[entryNode].data);

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
        float distNeighbour = distance(query, nodes[neighbour].data);

        if (topCandidates.size() < (size_t)ef ||
            distNeighbour < topCandidates.top().first) {
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
  /*
  FUNCTION:
    Prunes edges heuristically while maintaining spacial diversity.

  FUNCTION PARAMETERS:
    nodeId: integer, The index id of the node whose edges have to be pruned.
    layer: integer, The layer from of the graph from which the edges are being
  pruned.
  */

  std::vector<int> &neighbours = nodes[nodeId].neighbours[layer];

  // Layer 0 gets double capacity to prevent dead ends at the base
  int maxM = (layer == 0) ? M * 2 : M;

  // Check if neighbours already withing M limit
  if (neighbours.size() <= (size_t)maxM) return;

  // Cache distances and sort closest-first
  std::vector<std::pair<float, int>> candidates;
  for (int neighbour : neighbours) {
    float d = distance(nodes[nodeId].data, nodes[neighbour].data);
    candidates.push_back({d, neighbour});
  }
  std::sort(candidates.begin(), candidates.end());

  std::vector<int> keptNeighbours;
  std::vector<int> discardedNeighbours;

  // The Heuristic Check
  for (auto &candidate : candidates) {
    if (keptNeighbours.size() >= (size_t)maxM) break;

    int candidateId = candidate.second;
    float distToNode = candidate.first;
    bool goodEdge = true;

    // Check if this candidate is closer to ANY already kept neighbour than to
    // the base node
    for (int keptId : keptNeighbours) {
      float distToKept = distance(nodes[candidateId].data, nodes[keptId].data);
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

std::vector<std::pair<int, float>> HNSW::search(std::vector<float> &query,
                                                int k) {
  /*
  FUNCTION:
    Search for top-k candidates

  FUNCTION PARAMETERS:
    query: vector (float), The query vector to which we have to finc the most
  similar vectors. k: integer, The no. of top similar vector to return.
  */

  // Normalize the query vector since all the added vectors are normalized
  float sqr_sum = 0.0;
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

  int current = entryPoint;
  for (int i = maxLevel; i > 0; i--) {
    current = greedySearch(current, query, i);
  }

  auto topCandidates = searchLayer(query, current, 0, efSearch);
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
