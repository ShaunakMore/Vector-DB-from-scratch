#pragma once

#include<iostream>
#include<vector>
#include<cmath>
#include<string>
#include<stdexcept>
#include<map>
#include<random>
#include<chrono>
#include<queue>
#include<functional>
#include<fstream>

// Custom struct to store vector and vector id
struct VectorEntry{
    std::string id;
    std::vector<float>data;
};

float CosineSimilarity(const std::vector<float>& a, const std::vector<float>& b);

class ShoreDB {
public:
    std::vector<VectorEntry> VectorSpace;
    std::map<std::string, size_t> id_vector_map;

    const std::string characterString = "1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const size_t id_length = 16;

    std::mt19937 generator;
    std::uniform_int_distribution<int> random_index_gen;

    ShoreDB();

    std::string randomIDGenerator();

    bool addVector(const std::string& id, const std::vector<float>& dataVector);

    const VectorEntry* getVectorByID(const std::string& findID) const;

    std::vector<std::pair<std::string, float>> kNearestNeighbours(const std::vector<float>& query_vector, const int k);

    bool saveToDisk(const std::string& path = "ShoreDB") const;

    bool loadFromFile(const std::string& filename);
};