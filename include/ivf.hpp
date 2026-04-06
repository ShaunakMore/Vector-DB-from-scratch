#pragma once
#include <random>
#include <vector>

/**
 *@brief Hyperparameters required for the initialization of and IVF Index
 */
struct IVFHyperparams {
  int nlists = 100;         ///< Number of clusters(Inverted Lists).
  int nprob = 10;           ///< Number of clusters to search during query.
  int sample_size = 50000;  ///< Size of the training dataset to be used to train centroids.
  int max_vectors_per_cluster = 10000;  ///< Max number of vectors to be allowed per cluster.
  int rebuild_clusters = 50000;  ///< Number of vectors after which centroids should be retrained,
                                 ///< and clusters whould be rebuilt.
  int batch_size = 100;          ///< Mini-batch size for Mini-batch k-means
  std::mt19937 gen;
  std::uniform_real_distribution<float> dist;
};

class IVF {
 public:
  std::vector<std::vector<float>> centroids;  ///< Centroids around which clusters are built.
  std::vector<std::vector<int>>
      inverted_list;                        ///< Lists containing the mapping ClusterID -> VectorID.
  std::vector<std::vector<float>> vectors;  ///< Vetcors in the Index.

  /**
    *@brief Constructor to initialize the hyperparameters of the IVF index.
    *@param hyperParams Instance of the IVFHyperparameter class which contains al the initialised
     values of the hyperparameters.
    * Initializes all the required hyperparamters for the IVF Index. The values of these
      hyperparameters should be initialized thoughfully since it may strongly impact Iindex
    performance. of the index and results of search.
  */
  IVF(const IVFHyperparams &hyperParams);

  /**
   *@brief Trains centroids on sample data from the original dataset.
   *@param
   *@return Returns vector of trained centroids to be used for IVF.
   *Modifies the class variable centroids and reassigns it to the newly trained
    centroids. Frequency of training can be changed by changing the class
    variable rebuild_cluster. Uses Mini-batch spherical K-means for centroid training.
   */
  std::vector<std::vector<float>> trainCentroids();

  /**
   *@brief Inserts vectors inside IVF index
   *@param vec Input vector.
   * Normalizes the vector to be inserted using L2-Norm (Refer to normalize function for
     implementation details), then inserts the vector in the inverted list of the nearest centroid
     (cosine distance).
   */
  void insert(const std::vector<float> &vec);

  /**
   *@brief Searched for nearest vectors to query vector inside IVF index
   *@param query Query vector.
   *@return Top-k nearest vectors to query vector.
   */
  std::vector<std::vector<float>> search(const std::vector<float> &query);

  void testCreateIndex();

 private:
  IVFHyperparams params;

  /**
   *@brief Normalized the input vector using L2-Norm.
   *@param vec Input vector.
   *@return Returns L2-normalized vector.
   */
  void normaliseVector(std::vector<float> &vec);

  /**
    *@brief Implementation of Mini-Batch spherical k-means for training centroids.
    *@param training_reservoir Reservoir containing sample_size number of randomly sampled vectors
            from the original dataset.
    *@param centroid_reservoir Reservoir containing nlists number of randomly sampled vectors which
            act as the initial centroids for k-means.
    *@return Nothing.
    * The size of the batches used during the mini-batch k-means entirely depends on the
      hyperparameter batch_size, hence should be selected with care as it may impact performace and
      index creation time .

    *
  */
  void kMeans(std::vector<std::vector<float>> &training_reservoir,
              std::vector<std::vector<float>> &centroid_reservoir);
  /**
   *@brief Calculates cosine distance between the two NORMALIZED vectors.
   *@param vec1 NORMALIZED vector.
   *@param vec2 NORMALIZED vector.
   *@return Cosine distance between the two normalized vectors.
   *This function operates under the assumption that both the input vectors are normalized. If using
    exteranally, care should be taken that the input vector passed should be normalized to avoid
    mistakes.

   */
  float distance(const std::vector<float> &vec1, const std::vector<float> &vec2);
};
