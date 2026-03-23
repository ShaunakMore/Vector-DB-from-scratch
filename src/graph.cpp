#include "graph.hpp"
#include "distances.hpp"

#include<iostream>
#include<unordered_set>
#include<algorithm>

Node::Node(int assignedId,std::vector<float>vecData,int assignedLevel){
  id = assignedId;
  data = vecData;
  level = assignedLevel;
  neighbours.resize(level + 1);
  normaliseVector(data);

}

void Node::normaliseVector(std::vector<float>& vec){
  float sqr_sum = 0.0;
  for(float i:vec){
    sqr_sum += (i * i);
  }
  float mag = std::sqrt(sqr_sum);
  if(mag>0)
  {
    for(float& val:vec)
    {
      val/=mag;
    }
  }
}

HNSW::HNSW(int m,int efC,int efS){
  M = m;
  efConstruction = efC;
  efSearch = efS;
  mL = 1/log(M);
  gen = std::mt19937(std::random_device{}());
  dist = std::uniform_real_distribution<float>(std::numeric_limits<float>::epsilon(),1);

}
float HNSW::distance(const std::vector<float>&vec1,const std::vector<float>&vec2)
{
  /*
  FUNCTION:
    Calculates the L2 distance between the two vectors.
    
  FUNCTION PARAMETERS:
    vec1, vec2: vector (float), The two vectors whose L2 distance is to be calculated
  */
  
  float dot_prod = 0;
  for (size_t i = 0; i < vec1.size(); ++i) {
      dot_prod += vec1[i] * vec2[i];
  }
  return 1.0f - dot_prod; // Refer distances.cpp for implementation details
}

int HNSW::assignLevel()
{
  /*
  FUNCTION:
    Assigns a random level to a new node using a random, exponentially decaying function
    
  FUNCTION PARAMETERS:
   
  */

  int level = floor(-log(dist(gen)) * mL);
  return level; 
}


int HNSW::greedySearch(int entryNode, const std::vector<float>&queryVector,int level)
{
  /*
  FUNCTION:
    Greedy searches for the closest nodes starting from the entry node.
    
  FUNCTION PARAMETERS:
    entryNode: integer, The entry node from which the search begins for the layer.
    queryVector: vector (float), The vector for which we need to find the closest node
    level: integer, The level of the graph on which we are searching.
  */

  //Set the entry node as the current node
  //Better to reference using ids instead of passing Node objects
  int current = entryNode;
 
  float currentDist = distance(nodes[current].data, queryVector);

  while (true)
  {
      bool changed = false;

      std::vector<int>& neighbours = nodes[current].neighbours[level];

      for (int i : neighbours)
      {
          float neighbourDist = distance(nodes[i].data, queryVector);
          if (neighbourDist < currentDist)
          {
              current = i;
              currentDist = neighbourDist; // update cached distance
              changed = true;
          }
      }

      if (!changed) break;
  }
  return current;
}

void HNSW::insert(const std::vector<float>& vec)
{
  /*
  FUNCTION:
    Inserts a new node into the graph at the level assigned to it and connects it to other nodes.
    
  FUNCTION PARAMETERS:
    vec: vector (float), The vector data that is to be inserted

  */

  int id = nodes.size();
  int level = assignLevel();

  Node newNode = Node(id, vec, level);
  if (nodes.empty())
  {
      nodes.push_back(newNode);
      entryPoint = id;
      maxLevel = level;
      return;
  }
  nodes.push_back(newNode);

  int current = entryPoint;
  for (int layer = maxLevel; layer >= level + 1; layer--)
  {
      current = greedySearch(current, vec, layer);
  }

  int startLayer = std::min(level, maxLevel);

  for (int layer = startLayer; layer >= 0; layer--)
  {
    auto topCandidates = searchLayer(vec, current, layer, efConstruction);

    while (topCandidates.size() > M)
        topCandidates.pop();

    std::vector<int> neighbours;
    while (!topCandidates.empty())
    {
        neighbours.push_back(topCandidates.top().second);
        topCandidates.pop();
    }

    for (int i : neighbours)
        connectNodes(id, i, layer);

    if (neighbours.empty())
        break;

    current = neighbours.back();
  }

  if (level > maxLevel)
  {
      entryPoint = id;
      maxLevel = level;
  } 
}

std::priority_queue<std::pair<float,int>> HNSW::searchLayer(const std::vector<float>&query,int entryNode,int layer,int ef)
{
  /*
  FUNCTION:
    Searches the layer for the best possible ef number of candidates closest to query. Starts with neighbours of entryNode.

  FUNCTION PARAMETERS:
    query: vector (float), A reference to the query vector for which we need to find the closest neighbours
    entryNode: integer, The entry node to the layer where we begin our search
    layer: integer, The current layer of the graph we are searching on
    ef: integer, The hyperparameter efConstruction that decides the size of the candidate list
  */

  /*
    visited (set): Stores the nodes already visited
    candidatesQueue (min heap): stores the best candidates that can be visisted
    topCandidated (max heap): stores the topCandidates of neighbours
  */

  std::unordered_set<int> visited;
  std::priority_queue<std::pair<float,int>,std::vector<std::pair<float,int>>,std::greater<std::pair<float,int>>> candidateQueue;
  std::priority_queue<std::pair<float,int>,std::vector<std::pair<float,int>>> topCandidates;

  float entryNodeDist = distance(query,nodes[entryNode].data);

  candidateQueue.push(std::make_pair(entryNodeDist,entryNode));
  topCandidates.push(std::make_pair(entryNodeDist,entryNode));
  visited.insert(entryNode);

  while(!candidateQueue.empty())
  {
    auto current = candidateQueue.top();
    candidateQueue.pop();

    float currentDistance = current.first;
    int currentId = current.second;

    if (topCandidates.size() >= ef && currentDistance > topCandidates.top().first){
      break;
    }

    for(int neighbour : nodes[currentId].neighbours[layer])
    {
        if(visited.insert(neighbour).second)
        {
          float distNeighbour = distance(query, nodes[neighbour].data);

          if(topCandidates.size() < ef || distNeighbour < topCandidates.top().first)
          {
            candidateQueue.push({distNeighbour, neighbour});
            topCandidates.push({distNeighbour, neighbour});

            if(topCandidates.size() > ef)
            {
              topCandidates.pop();
            }   
          }
        }
    }
  }
  return topCandidates;
}

void HNSW::connectNodes(int node1,int node2,int layer)
{
  /*
  FUNCTION:
    Connects two nodes using bidirectional edges. Also prunes the edges if edges exceed M (max edeges possible).

  FUNCTION PARAMETERS:
    node1, node2: integer, Index Ids of the two nodes to be connected
    layer: integer, The layer on which the connections are to be made
  */

  nodes[node1].neighbours[layer].push_back(node2);
  nodes[node2].neighbours[layer].push_back(node1);

  pruneNeighbours(node1,layer);
  pruneNeighbours(node2,layer);
}

void HNSW::pruneNeighbours(int nodeId,int layer)
{
  /*
  FUNCTION:
    Prunes the graph by removing the frathest edges if number of edges exceed M.

  FUNCTION PARAMETERS:
    nodeId: integer, Index Id of the node to be checked if it should be pruned or not.
    layer: integer, The layer on which edges should be pruned.
  */

  std::vector<int>& neighbours = nodes[nodeId].neighbours[layer];
 
  if (neighbours.size() < (size_t)M) return;

  std::priority_queue<
      std::pair<float, int>,
      std::vector<std::pair<float, int>>,
      std::greater<std::pair<float, int>>> neighbourQueue;

  for (int neighbour : neighbours)
  {
      float d = distance(nodes[neighbour].data, nodes[nodeId].data);
      neighbourQueue.push({d, neighbour});
  }

  std::vector<int> keepNeighbours;
  int kept = 0;
  while (!neighbourQueue.empty())
  {
      if (kept < M)
      {
          keepNeighbours.push_back(neighbourQueue.top().second);
          kept++;
      }
      else{
        break;
      }
      neighbourQueue.pop();
  }

  neighbours = std::move(keepNeighbours); // now replace the list

}

void HNSW::removeNeighbour(int nodeId,int neighbourId, int layer)
{
  /*
  FUNCTION:
    Removes the bidirectional edges form the pruned edges from pruneNeighbours.
      
  FUNCTION PARAMETERS:
    nodeId: integer, The index id of the edge that has been pruned in pruneNeighbours.
    neighbourId: integer, The index id of the edge to which the connection is to be severed.
    layer: integer, The layer from of the graph from which the edges are being pruned.
  */

  std::vector<int>& neighbours = nodes[nodeId].neighbours[layer];
  auto it = std::remove(neighbours.begin(),neighbours.end(),neighbourId);
  neighbours.erase(it,neighbours.end());
}

std::vector<std::pair<int,float>> HNSW::search(std::vector<float>& query,int k){
  /*
  FUNCTION:
    Removes the bidirectional edges form the pruned edges from pruneNeighbours.
      
  FUNCTION PARAMETERS:
    nodeId: integer, The index id of the edge that has been pruned in pruneNeighbours.
    neighbourId: integer, The index id of the edge to which the connection is to be severed.
    layer: integer, The layer from of the graph from which the edges are being pruned.
  */
 
  float sqr_sum = 0.0;
  for(float i : query){
      sqr_sum += (i * i);
  }
  float mag = std::sqrt(sqr_sum);
  if(mag > 0){
      for(float& val : query){
          val /= mag;
      }
  }
  std::vector<std::pair<int,float>>topK;

  if(nodes.empty())
  { 
    return topK;
  }

  int current = entryPoint;
  for(int i = maxLevel;i>0;i--)
  {
    current = greedySearch(current,query,i);
  }

  auto topCandidates = searchLayer(query,current,0,efSearch);
  while(topCandidates.size()> (size_t)k)
  {
    topCandidates.pop();
  }
  while(!topCandidates.empty())
  {
    topK.push_back(std::make_pair(topCandidates.top().second,topCandidates.top().first));
    topCandidates.pop();
  }
  
  std::reverse(topK.begin(),topK.end());
  return topK;
}
