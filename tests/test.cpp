#include "../include/shore_db.hpp"
#include <iostream>

int main() {
    try {
        ShoreDB db;

        // Sample data
        std::vector<float> vec1 = {1.0, 0.0, 0.0};
        std::vector<float> vec2 = {0.0, 1.0, 0.0};
        std::vector<float> vec3 = {0.0, 0.0, 1.0};
        std::vector<float> vec4 = {1.0, 1.0, 0.0};

        // Add vectors
        db.addVector("vec1", vec1);
        db.addVector("vec2", vec2);
        db.addVector("vec3", vec3);
        db.addVector("vec4", vec4);

        std::cout << "✅ Vectors added successfully.\n";

        // Get vector by ID
        const VectorEntry* fetched = db.getVectorByID("vec2");
        std::cout << "🔍 Retrieved vector 'vec2': ";
        for (auto val : fetched->data) std::cout << val << " ";
        std::cout << "\n";

        // Perform KNN Search
        std::vector<float> query = {0.9, 0.9, 0.0};
        int k = 2;
        auto results = db.kNearestNeighbours(query, k);

        std::cout << "🔎 Top " << k << " nearest vectors to [0.9, 0.9, 0.0]:\n";
        for (const auto& [id, score] : results) {
            std::cout << "   ID: " << id << " | Cosine Similarity: " << score << "\n";
        }

        // Save to disk
        db.saveToDisk("TestVectorDB");
        std::cout << "💾 Saved database to 'TestVectorDB'\n";

        // Load into a new instance
        ShoreDB dbLoaded;
        dbLoaded.loadFromFile("TestVectorDB");
        std::cout << "📂 Loaded database from 'TestVectorDB'\n";

        // Test retrieval from loaded DB
        const VectorEntry* loadedVec = dbLoaded.getVectorByID("vec4");
        std::cout << "🔄 Loaded vector 'vec4': ";
        for (float f : loadedVec->data) std::cout << f << " ";
        std::cout << "\n";

    } catch (const std::exception& e) {
        std::cerr << "❌ Exception: " << e.what() << std::endl;
    }

    return 0;
}