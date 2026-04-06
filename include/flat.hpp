#pragma once

#include <map>
#include <queue>
#include <random>
#include <string>
#include <vector>

// Custom struct to store vector and vector id
struct VectorEntry {
  std::string id;
  std::vector<float> data;
  bool deleted = false;
};

class Flat {
 public:
  std::vector<VectorEntry> VectorSpace;
  std::map<std::string, size_t> id_vector_map;
  std::queue<int> freeSlots;
  bool set_auto_cleanup = true;
  float cleanup_threshold = 0.3;

  const std::string characterString =
      "1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
  const size_t id_length = 16;
  std::mt19937 generator;
  std::uniform_int_distribution<int> random_index_gen;

  Flat();

  std::string randomIDGenerator();

  bool addVector(const std::string& id, const std::vector<float>& dataVector);

  const VectorEntry* getVectorByID(const std::string& findID) const;

  std::vector<std::pair<std::string, float>> kNearestNeighbours(
      const std::vector<float>& query_vector, const int k);

  bool saveToDisk(const std::string& path = "ShoreDB") const;

  bool loadFromFile(const std::string& filename);

  bool deleteVector(const std::string id);

  bool quickDelete(const std::string id);

  bool removeDeletedVectors();

  float getStats();

  void autoCleanup();
};