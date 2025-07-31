# SimpleVectorDB 🚀

A lightweight, header-only C++ vector database implementation for similarity search using cosine similarity. Perfect for prototyping, embedding storage, and small to medium-scale similarity search applications.

## ✨ Features

- **Fast Similarity Search**: K-nearest neighbors search using cosine similarity
- **Simple API**: Easy-to-use interface for adding, retrieving, and searching vectors
- **Memory Efficient**: In-memory storage with efficient data structures
- **Robust Error Handling**: Comprehensive exception handling for edge cases
- **Random ID Generation**: Automatic unique ID generation for vectors
- **Zero Dependencies**: Pure C++ implementation using only standard library

## 🛠️ Requirements

- C++11 or later
- Standard C++ library (no external dependencies)

## 🚀 Quick Start

### Basic Usage

```cpp
#include "SimpleVectorDB.h"

int main() {
    // Initialize the database
    SimpleVectorDB db;
    
    // Add some vectors
    db.addVector("doc1", {1.0f, 0.5f, 0.2f, 0.8f});
    db.addVector("doc2", {0.9f, 0.6f, 0.1f, 0.7f});
    db.addVector("doc3", {0.1f, 0.9f, 0.8f, 0.2f});
    
    // Search for similar vectors
    std::vector<float> query = {1.0f, 0.4f, 0.3f, 0.9f};
    auto results = db.kNearestNeighbours(query, 2);
    
    // Print results
    for (const auto& result : results) {
        std::cout << "ID: " << result.first 
                  << ", Similarity: " << result.second << std::endl;
    }
    
    return 0;
}
```

### Compilation

```bash
g++ -std=c++11 -O2 your_program.cpp -o your_program
```

## 📖 API Reference

### Core Methods

#### `addVector(const std::string& id, const std::vector<float>& dataVector)`
Adds a new vector to the database.
- **Parameters**: 
  - `id`: Unique identifier for the vector
  - `dataVector`: The vector data as a list of floats
- **Throws**: `std::invalid_argument` if ID already exists
- **Returns**: `bool` (always true on success)

#### `getVectorByID(const std::string& findID)`
Retrieves a vector by its ID.
- **Parameters**: `findID`: The ID to search for
- **Returns**: `const VectorEntry*` pointer to the vector entry
- **Throws**: `std::invalid_argument` if ID doesn't exist

#### `kNearestNeighbours(const std::vector<float>& query_vector, const int k)`
Finds the k most similar vectors to the query vector.
- **Parameters**: 
  - `query_vector`: The vector to search for
  - `k`: Number of nearest neighbors to return
- **Returns**: `std::vector<std::pair<std::string, float>>` - pairs of (ID, similarity_score)
- **Throws**: Various exceptions for invalid inputs

#### `randomIDGenerator()`
Generates a unique random ID for vectors.
- **Returns**: `std::string` - a 16-character random ID
- **Note**: Automatically ensures uniqueness within the database

### Utility Functions

#### `CosineSimilarity(const std::vector<float>& a, const std::vector<float>& b)`
Calculates cosine similarity between two vectors.
- **Returns**: `float` between -1 and 1 (1 = identical, 0 = orthogonal, -1 = opposite)
- **Throws**: `std::runtime_error` for dimension mismatch or zero-length vectors

## 🎯 Use Cases

- **Document Similarity**: Compare text embeddings for semantic search
- **Recommendation Systems**: Find similar items based on feature vectors  
- **Image Search**: Compare image feature vectors for visual similarity
- **Data Analysis**: Cluster and find patterns in high-dimensional data
- **Prototyping**: Quick setup for ML/AI projects requiring similarity search

## ⚡ Performance Characteristics

- **Time Complexity**: 
  - Insert: O(1) average case
  - Search: O(n×d) where n = number of vectors, d = vector dimensions
  - Retrieval by ID: O(log n)
- **Space Complexity**: O(n×d) for storing vectors
- **Suitable for**: Up to ~100K vectors with reasonable performance

## 🧪 Testing

The included test suite covers:
- ✅ Vector addition and duplicate ID handling
- ✅ Vector retrieval by ID
- ✅ K-nearest neighbors search
- ✅ Error handling for edge cases
- ✅ Zero vector handling
- ✅ Random ID generation

Run the tests:
```bash
g++ -std=c++11 -O2 SimpleVectorDB.cpp -o test_db
./test_db
```

## 🔮 Future Goals

Here's what's next on the roadmap:
* 💾 **On-Disk Persistence** Load/save vectors using binary serialization or memory-mapped files.
* ⚙️ **Approximate Nearest Neighbor (ANN) Search** Speed things up with LSH, KD-Trees, or other ANN techniques.
* 🧵 **Multithreaded Search & Insertion** Thread-safe operations using mutexes or concurrent data structures.
* 🧠 **Custom Distance Metrics** Add support for Euclidean, Manhattan, etc.

## 🤝 Contributing

Contributions are welcome! Here are some ways you can help:
- 🐛 Report bugs and edge cases
- 💡 Suggest new features
- 🔧 Submit performance improvements
- 📚 Improve documentation
- ✨ Add more distance metrics

## 📝 License

This project is open source. Feel free to use, modify, and distribute as needed.

## 🙏 Acknowledgments

Built with modern C++ best practices and designed for simplicity and performance. Perfect for educational purposes, prototyping, and small to medium-scale applications.

---

**Happy vector searching!** 🔍✨