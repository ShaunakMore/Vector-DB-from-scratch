#pragma once
#include<vector>

class Node
{
  /* data */
  public:
    int id;
    std::vector<float>data;
    int level;
    std::vector<std::vector<int>>neighbors;
    Node(int assignedId,std::vector<float>vecData,int assignedLevel);
};

class HNSW{
  public:
    std::vector<Node> nodes;
    int entryPoint;
    int maxLevel;
    int M;
    int efConstruction;
    float mL;
    
    HNSW(int m,int efC);

    void insert(const std::vector<float>& vec);
    std::vector<float> search(const std::vector<float>& query,int k);
    

  private:

    float distance(const std::vector<float>&vec1,const std::vector<float>&vec2);
    int assignLevel();
    int greedySearch(int entryNode, const std::vector<float>&queryVector,int level);
    std::vector<int>searchLayer(const std::vector<float>&query,int entryNode,int layer,int ef);
    void connectNodes(int node1,int node2,int layer);
    
};
