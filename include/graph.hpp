#pragma once
#include <queue>
#include <random>
#include <vector>

/**
 *@brief Node class which stores graph node data.
 */
class Node {
  /* data */
 public:
  int id;     ///< Node id
  int level;  ///< The highest level of graph on which the node exists
  std::vector<std::vector<int>> neighbours;  ///< List of ids all the neighbours of the Node at
                                             ///< every level upto its highest level
  Node(int assignedId, int assignedLevel);
};

class HNSW {
 public:
  int DIM;  ///< Dimension of the input vectors

  std::vector<float>
      flat_storage;  ///< A flat storage which stores all input vectors in a linear fashion

  std::vector<Node> nodes;  ///< List of all the nodes in the graph

  std::vector<int>
      visited;  ///< Used for keeping track of visited nodes using versioning to improve
                ///< greedySearch performance. Refer greedy search implementation for more info.

  int search_version;  ///< Tracks search version

  int entryPoint;  ///< Entry point to the NSW graph

  int maxLevel;  ///< The maximum level in the graph

  int M;

  int efConstruction;  ///< Max size of the candidate list during graph construction

  int efSearch;  ///< Max size of the candidate list during graph search

  float mL;
  std::mt19937 gen;
  std::uniform_real_distribution<float> dist;

  HNSW(int m, int efC, int efS, int dim);

  /**
   *@brief A method that returns a pointer to the vector data using its id.
   *@param id The id of the vector we want to retrieve.
   *@return Pointer to query vector.
   */
  const float* get_vector(int id);

  /**
   *@brief Mutable form of the get_vector method.
   */
  float* get_vector_mutable(int id);

  /**
   *@brief Inserts node into the HNSW Index and vector into flat storage.
   *@param vec Input vector.
   *@return Nothing.
   *Stores the vector data into a flat storage and creates an instance of the Node class to add the
   *Node to the Index.
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
  void normaliseVector(float* vecStart, int DIM);

  /**
   *@brief Computes cosine distance.
   *@param vec1 Pointer to Input vector 1.
   *@param vec2 Pointer to Input vector 2.
   *@return Cosine distance between the two vectors.
   */
  float distance(const float* vec1_start, const float* vec2_start);

  /**
   *@brief Assigns a random level to a new node using a random, exponentially decaying function.
   *@param
   *@return Level at which node should be inserted.
   */
  int assignLevel();

  /**
   *@brief Greedily searches for the closest nodes starting from the entry node.
   *@param entryNode The entry node from which the search begins for the layer.
   *@param queryVector Pointer to the vector for which we need to find the closest node.
   *@param level The level of the graph on which we are searching.
   *@return Closest node to queryVector on the provided level.
   *Nodes are referenced using their ids instead of returning the complete Node object, makes it
   *easy to access Node data and also improves performance.
   */
  int greedySearch(int entryNode, const float* queryVecStart, int level);

  /**
   *@brief Searches the layer for the best possible ef number of candidates closest to query. Starts
           with neighbours of entryNode.
   *@param query Pointer to the query vector for which we need to find the closest neighbours.
   *@param entryNode The entry node to the layer where we begin our search.
   *@param layer The current layer of the graph we are searching on.
   *@param ef The hyperparameter efConstruction that decides the size of the candidate list.
   *@return A priority queue containing (distance, id) pairs of top ef number of candidates.
   */
  std::priority_queue<std::pair<float, int>> searchLayer(const float* query, int entryNode,
                                                         int layer, int ef);

  /**
   *@brief Prunes edges heuristically while maintaining spacial diversity.
   *@param nodeId index id of the node whose edges have to be pruned.
   *@param layer The layer from of the graph from which the edges are being.
   *@return Nothing.
   */
  void pruneNeighbours(int nodeId, int layer);
};
