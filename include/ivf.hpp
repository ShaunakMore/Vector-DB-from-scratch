#pragma once
#include <random>
#include <vector>

struct IVFHyperparams {
  int nlists;
  int nprob;
  int sample_size;
  int max_vectors_per_cluster;
  int rebuild_clusters;
  int batch_size;
  std::mt19937 gen;
  std::uniform_real_distribution<float> dist;
};

class IVF {
 public:
  std::vector<std::vector<float>> centroids;
  std::vector<std::vector<int>> inverted_list;
  std::vector<std::vector<float>> vectors;

  IVF(const IVFHyperparams &hyperParams);
  /**
   *@brief Trains centroids on sample data from the original dataset
   *@param
   *@return Returns vector of trained centroids to be used for IVF
   */
  std::vector<std::vector<float>> trainCentroids();

  /**
   *@brief Inserts vectors inside IVF index
   *@param Input vector
   *@return
   */
  void insert(const std::vector<float> &vec);

  /**
   *@brief Searched for nearest vectors to query vector inside IVF index
   *@param Input vector
   *@return Top-k nearest vectors to query vector
   */
  std::vector<std::vector<float>> search(const std::vector<float> &query);

 private:
  IVFHyperparams params;
  /**
   *@brief Normalized the input vector on magnitude
   *@param Input vector
   *@return Returns normalized vector
   */
  void normaliseVector(std::vector<float> &vec);
  void kMeans(std::vector<std::vector<float>> &training_reservoir,
              std::vector<std::vector<float>> &centroid_reservoir);
  /**
   *@brief Calculates cosine distance between the two NORMALIZED vectors
   *@param Normalized vectors vec1, vec2
   *@return Cosine distance between the two normalized vectors
   */
  float distance(const std::vector<float> &vec1,
                 const std::vector<float> &vec2);
};

void addInplace(std::vector<float> &a, std::vector<float> &b);
void scalarProd(const int a, std::vector<float> &vec);
