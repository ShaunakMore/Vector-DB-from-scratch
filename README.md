# 🧠 ShoreDB

A lightweight, high-performance vector database built in C++ for efficient similarity search and vector storage.

## Overview

ShoreDB is designed to be a simple yet powerful vector database that provides fast nearest neighbor search capabilities. Built with performance and ease of use in mind, it's perfect for applications requiring semantic search, recommendation systems, or any scenario where you need to find similar vectors quickly.

## Features

- **Fast Vector Operations**: Optimized add, delete, and search operations
- **Flexible Deletion Strategy**: Supports both soft deletes with rollback and immediate deletion
- **Efficient Memory Management**: Smart swap-and-pop mechanism prevents fragmentation
- **Persistent Storage**: Save and load your vector database to/from disk
- **Simple API**: Clean, intuitive interface for all operations
- **ID-based Indexing**: Quick O(1) lookup by vector ID

## Key Design Principles

### Deletion Strategies

ShoreDB offers two deletion modes to suit different use cases:

**Soft Delete (Default)**
- Marks vectors as deleted without immediately removing them
- Supports rollback/undo operations
- Skips deleted vectors during search operations
- Batch cleanup for optimal performance

**Immediate Delete**
- Instant removal using swap-and-pop algorithm
- No rollback support
- Zero fragmentation overhead

### Memory Optimization

Uses a sophisticated swap-and-pop algorithm during cleanup that:
- Maintains O(1) deletion performance
- Prevents vector fragmentation
- Keeps memory usage compact
- Preserves index integrity

## Project Structure

```
.
├── include/
│   └── shore_db.hpp         # Core header with class declaration
├── src/
│   └── shore_db.cpp         # Implementation file
├── tests/
│   └── test.cpp             # Simple test runner (acts as main)
├── Makefile                 # For building the project
└── README.md
```

## Installation

```bash
# Clone the repository
git clone https://github.com/yourusername/shoredb.git
cd shoredb

# Build (example using g++)
g++ -std=c++17 -O3 -o shoredb_example example.cpp shoredb.cpp
```

## Quick Start

```cpp
#include "shoredb.h"

// Create a new database
ShoreDB db;

// Add vectors
std::vector<float> vec1 = {1.0, 2.0, 3.0};
db.add_vector(1, vec1);

std::vector<float> vec2 = {1.1, 2.1, 3.1};
db.add_vector(2, vec2);

// Search for similar vectors
std::vector<float> query = {1.0, 2.0, 3.0};
auto results = db.search(query, 5);  // Get top 5 nearest neighbors

// Delete a vector (soft delete)
db.delete_vector(1);

// Clean up deleted vectors
db.remove_deleted_vectors();

// Save to disk
db.save_to_disk("my_vectors.db");

// Load from disk
ShoreDB loaded_db;
loaded_db.load_from_disk("my_vectors.db");
```

## API Reference

### Core Operations

**`bool ShoreDB::addVector(const std::string& id, const std::vector<float>& dataVector)`**
- Adds a new vector to the database
- Each vector must have a unique ID

**`bool deleteVector(const std::string id);`**
- Soft deletes a vector (marks as deleted)
- Supports rollback
- Vector is removed during cleanup

**`bool quickDelete(const std::string id)`**
- Immediately removes vector from database
- No rollback support
- Uses swap-and-pop for O(1) performance

**`const VectorEntry* getVectorByID(const std::string& findID)`**
- Finds k nearest neighbors to the query vector
- Returns results sorted by similarity
- Automatically skips deleted vectors

### Maintenance Operations

**`bool removeDeletedVectors()`**
- Physically removes all soft-deleted vectors
- Compacts the database
- Should be called periodically or before saving

**`float getStats()`**
- Returns statistics about database state
- Shows fragmentation ratio
- Helps decide when to compact

**`void autoCleanup()`**
- Auto cleans memory by removing softly deleted vectors
- Only cleans when auto clean flag is set and threshold is exceeded

### Persistence

**`void save_to_disk(const std::string& path)`**
- Saves database to disk
- Automatically cleans up deleted vectors before saving

**`void load_from_disk(const std::string& path)`**
- Loads database from disk
- Overwrites current database state

## Performance Characteristics

| Operation | Time Complexity | Notes |
|-----------|----------------|-------|
| Add | O(1) | Amortized |
| Search | O(n·d) | n=vectors, d=dimensions |
| Delete (soft) | O(1) | Immediate flag set |
| Delete (immediate) | O(1) | Swap-and-pop |
| Cleanup | O(n) | Only deleted vectors processed |

## Roadmap

- [ ] HNSW indexing for faster search
- [ ] IVF (Inverted File Index) support
- [ ] Multi-threaded search
- [ ] Compressed vector storage
- [ ] Filtering support during search
- [ ] Custom distance metrics

## Use Cases

- **Semantic Search**: Find similar documents, images, or text
- **Recommendation Systems**: Content-based filtering
- **Duplicate Detection**: Identify similar items
- **Clustering**: Group similar vectors together
- **Anomaly Detection**: Find outliers in vector space

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.


## Author

Shaunak More
Built with ☕, curiosity, and late-night debugging.

---

## 📝 Notes: 
* ShoreDB is currently in active development. APIs may change between versions.
* ShoreDB currently only uses **Euclidean distance** for similarity.
* This is a basic implementation aimed at understanding the internals of vector DBs, not optimized for production use.
* ANN is implemented using a priority queue approach (brute-force for now).
