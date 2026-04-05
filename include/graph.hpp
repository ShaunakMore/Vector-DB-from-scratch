#pragma once
#include <queue>
#include <random>
#include <vector>

class Node {
  /* data */
 public:
  int id;
  std::vector<float> data;
  int level;
  std::vector<std::vector<int>> neighbours;
  Node(int assignedId, std::vector<float> vecData, int assignedLevel);
  void normaliseVector(std::vector<float>& vec);
};

class HNSW {
 public:
  std::vector<Node> nodes;
  std::vector<int> visited;
  int search_version;

  int entryPoint;
  int maxLevel;
  int M;
  int efConstruction;
  int efSearch;
  float mL;
  std::mt19937 gen;
  std::uniform_real_distribution<float> dist;

  HNSW(int m, int efC, int efS);

  void insert(const std::vector<float>& vec);
  std::vector<std::pair<int, float>> search(std::vector<float>& query, int k);

 private:
  float distance(const std::vector<float>& vec1,
                 const std::vector<float>& vec2);
  int assignLevel();
  int greedySearch(int entryNode, const std::vector<float>& queryVector,
                   int level);
  std::priority_queue<std::pair<float, int>> searchLayer(
      const std::vector<float>& query, int entryNode, int layer, int ef);
  void pruneNeighbours(int nodeId, int layer);
};
