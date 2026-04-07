#pragma once
#include <queue>
#include <random>
#include <vector>

/**
 *@brief Node class whjich stores graph node details and handles normalization.
 *The normalizeVector method normalizes the Node vector on Node creation  using L2-Norm. This should
  be taken into consideration while considering similarity search. The distance calculated after
  L2-Norm is cosine distance. Refer distance method implementation for more info.
 */
class Node {
  /* data */
 public:
  int id;                   ///< Node id
  std::vector<float> data;  ///< Vector data
  int level;                ///< The highest level of graph on which the node exists
  std::vector<std::vector<int>>
      neighbours;  ///< List of all the neighbours of the Node at every level upto its highest level
  Node(int assignedId, std::vector<float> vecData, int assignedLevel);
  void normaliseVector(std::vector<float>& vec);  ///< Normalize the Node vector data
};

class HNSW {
 public:
  std::vector<Node> nodes;  ///< List of all the nodes in the graph
  std::vector<int>
      visited;  ///< Used for keeping track of visited nodes using versioning to improve
                ///< greedySearch performance. Refer greedy search implementation for more info.
  int search_version;  ///< Tracks search version

  int entryPoint;  ///< Entry point to the NSW graph
  int maxLevel;    ///< The maximum level in the graph
  int M;
  int efConstruction;  ///< Max size of the candidate list during graph construction
  int efSearch;        ///< Max size of the candidate list during graph search
  float mL;
  std::mt19937 gen;
  std::uniform_real_distribution<float> dist;

  HNSW(int m, int efC, int efS);
  /**
   *@brief Inserts node into the HNSW Index.
   *@param vec Input vector.
   *@return Nothing.
   *Creates an instance of the Node class to store the input vector.
   */
  void insert(const std::vector<float>& vec);

  /**
   *@brief Searches the index and returns the top-k results.
   *@param query Query vector
   *@param k Number of top results to return (top-k)
   *@return Vector containg top-k results as a (id, distance form query) pair
   */
  std::vector<std::pair<int, float>> search(std::vector<float>& query, int k);

 private:
  /**
   *@brief Normalizes the vector to reduce distance computation.
   *@param vec1 Input vector 1.
   *@param vec2 Input vector 2.
   *@return Cosine distance between the two vectors.
   *This method runs on every new Node creation through the Node constructor.
   */
  float distance(const std::vector<float>& vec1, const std::vector<float>& vec2);

  /**
   *@brief Assigns a random level to a new node using a random, exponentially decaying function.
   *@param
   *@return Level at which node should be inserted.
   */
  int assignLevel();

  /**
   *@brief **Greedily** searches for the closest nodes starting from the entry node.
   *@param entryNode The entry node from which the search begins for the layer.
   *@param queryVector The vector for which we need to find the closest node.
   *@param level The level of the graph on which we are searching.
   *@return Closest node to queryVector on the provided level.
   *Nodes are referenced using their ids instead of returning the complete Node object, makes it
   *easy to access Node data and also improves performance.
   */
  int greedySearch(int entryNode, const std::vector<float>& queryVector, int level);

  /**
   *@brief Searches the layer for the best possible ef number of candidates closest to query. Starts
           with neighbours of entryNode.
   *@param query A reference to the query vector for which we need to find the closest neighbours.
   *@param entryNode The entry node to the layer where we begin our search.
   *@param layer The current layer of the graph we are searching on.
   *@param ef The hyperparameter efConstruction that decides the size of the candidate list.
   *@return A priority queue containing (distance, id) pairs of top ef number of candidates.
   */
  std::priority_queue<std::pair<float, int>> searchLayer(const std::vector<float>& query,
                                                         int entryNode, int layer, int ef);

  /**
   *@brief Prunes edges heuristically while maintaining spacial diversity.
   *@param nodeId index id of the node whose edges have to be pruned.
   *@param layer The layer from of the graph from which the edges are being.
   *@return Nothing.
   */
  void pruneNeighbours(int nodeId, int layer);
};
