#include "ivf.hpp"

#include <algorithm>
#include <climits>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <ostream>
#include <random>
#include <vector>

#include "vector_ops.hpp"

IVF::IVF(const IVFHyperparams &hyperparams) {
  params = hyperparams;
  params.gen = std::mt19937(42);
  params.dist = std::uniform_real_distribution<float>(0.0f, 1.0f);
  inverted_list.assign(params.nlists, std::vector<int>{});
}

float IVF::distance(const std::vector<float> &vec1, const std::vector<float> &vec2) {
  float dot_prod = 0;
  for (size_t i = 0; i < vec1.size(); ++i) {
    dot_prod += vec1[i] * vec2[i];
  }
  return 1.0f - dot_prod;
}

void IVF::normaliseVector(std::vector<float> &vec) {
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
void IVF::insert(const std::vector<float> &vec) {
  int idx = vectors.size();
  vectors.push_back(vec);
  normaliseVector(vectors[idx]);
  if ((int)vectors.size() % params.rebuild_clusters) {
    centroids = trainCentroids();
  }
  float minDist = INFINITY;
  int closestCentroid = 0;
  for (size_t centroid = 0; centroid < centroids.size(); centroid++) {
    float currDist = distance(vectors[idx], centroids[centroid]);
    if (currDist < minDist) {
      minDist = currDist;
      closestCentroid = (int)centroid;
    }
  }
  inverted_list[closestCentroid].push_back(idx);
}

void IVF::testCreateIndex() {
  centroids = trainCentroids();
  std::cout << "Indexing " << vectors.size() << " vectors ..." << std::endl;
  for (size_t vectorId = 0; vectorId < vectors.size(); vectorId++) {
    float minDist = INFINITY;
    int closestCentroid = 0;
    for (size_t centroid = 0; centroid < centroids.size(); centroid++) {
      float currDist = distance(vectors[vectorId], centroids[centroid]);
      if (currDist < minDist) {
        minDist = currDist;
        closestCentroid = centroid;
      }
    }
    inverted_list[closestCentroid].push_back(vectorId);
  }
  for (int i = 0; i < (int)inverted_list.size(); i++) {
    std::cout << "Centroid No. " << i << ", No. of vectors = " << inverted_list[i].size()
              << std::endl;
  }
}
std::vector<std::vector<float>> IVF::trainCentroids() {
  std::cout << "Training centroids..." << std::endl;

  /*
    A reservoir to store vectors used to calculate the initial centroids. These are sampled randomly
    using reservoir sampling
  */
  std::vector<std::vector<float>> training_reservoir(params.sample_size);
  std::cout << "Creating Training Reservoir of size " << params.sample_size << " ..." << std::endl;

  for (int i = 0; i < params.sample_size; i++) training_reservoir[i] = vectors[i];

  std::cout << "Training reservoir created of size " << int(training_reservoir.size()) << "!"
            << std::endl;

  // Use reservoir sampling to sample the vectors in the training reservoir
  for (size_t i = (size_t)params.sample_size; i < vectors.size(); i++) {
    int j = std::floor(i * params.dist(params.gen));
    if (j < params.sample_size) {
      training_reservoir[j] = vectors[i];
    }
  }

  std::cout << "Shuffling training reservoir... !" << std::endl;

  /*
    Shuffle vectors sampled in the reservoir to select nlists number of vectors which act as
    initial centroids
  */
  std::shuffle(training_reservoir.begin(), training_reservoir.end(), params.gen);

  /*
    A reservoir for the centroids which will be trained using k-means.
  */
  std::vector<std::vector<float>> centroids_reservoir(params.nlists);

  std::cout << "Selecting random centroids..." << std::endl;
  for (int i = 0; i < params.nlists; i++) {
    centroids_reservoir[i] = training_reservoir[i];
  }
  std::cout << "Starting K-means on the random centroids of size " << centroids_reservoir.size()
            << " !" << std::endl;

  /*
    Apply k-means on the vectors in the training reservoir and the initial centroids
  */
  kMeans(training_reservoir, centroids_reservoir);
  std::cout << "Completed K-means!!" << std::endl;
  return centroids_reservoir;
}

void IVF::kMeans(std::vector<std::vector<float>> &training_reservoir,
                 std::vector<std::vector<float>> &centroid_reservoir) {
  /*
    Temporary index that stores cluster information during k-means iteration.
  */
  std::vector<std::vector<int>> sampleIndex(centroid_reservoir.size(), std::vector<int>{});

  /*
    A counter that keeps track of all the vectors seen by a centroid after every iteratin of
    k-means. Important for weighted avg. calculation of centroids.
  */
  std::vector<int> counter(centroid_reservoir.size(), 0);

  for (int i = 0; i < params.max_iters; i++) {
    std::cout << "K-means iteration number: " << i << std::endl;

    std::shuffle(training_reservoir.begin(), training_reservoir.end(), params.gen);

    for (size_t start = 0; start < training_reservoir.size(); start += params.batch_size) {
      size_t end = std::min(start + params.batch_size, training_reservoir.size());

      for (auto &v : sampleIndex) v.clear();

      std::cout << "Processing vectors " << start << " to " << int(end) << "..." << std::endl;
      for (size_t vectorId = start; vectorId < end; vectorId++) {
        size_t closestCentroid = 0;
        float minDist = INFINITY;

        for (size_t centroidId = 0; centroidId < centroid_reservoir.size(); centroidId++) {
          float dist = distance(training_reservoir[vectorId], centroid_reservoir[centroidId]);

          if (dist < minDist) {
            minDist = dist;
            closestCentroid = centroidId;
          }
        }
        sampleIndex[closestCentroid].push_back(vectorId);
        counter[closestCentroid] += 1;
      }

      for (size_t cluster = 0; cluster < sampleIndex.size(); cluster++) {
        if (sampleIndex[cluster].empty()) continue;

        std::vector<float> meanCentroid(training_reservoir[0].size(), 0.0f);
        for (auto vectorId : sampleIndex[cluster]) {
          addInplace(meanCentroid, training_reservoir[vectorId]);
        }

        float inv = 1.0f / sampleIndex[cluster].size();
        scalarProd(inv, meanCentroid);

        float lr = 1.0f / counter[cluster];

        scalarProd(1.0f - lr, centroid_reservoir[cluster]);
        scalarProd(lr, meanCentroid);
        addInplace(centroid_reservoir[cluster], meanCentroid);

        normaliseVector(centroid_reservoir[cluster]);
      }
    }
  }
}
