
---

````markdown
# 🔍 SimpleVectorDB

A minimal, fast, and fully in-memory (for now!) **vector database** written in **modern C++**, built from scratch with zero external dependencies.  
Currently supports exact k-Nearest Neighbors using Cosine Similarity. Plans underway for **disk storage**, **ANN search**, and much more.

> Think of it as a baby Pinecone / FAISS... but handcrafted in pure C++.

---

## 🚀 Features

- ✅ Add & retrieve vectors by unique ID
- ✅ k-Nearest Neighbor search (Cosine Similarity)
- ✅ Random ID generation with collision handling
- ✅ Zero-vector detection & exception-safe search
- ✅ Priority queue for efficient top-K retrieval
- ✅ In-memory map for O(1) lookups
- ✅ Fully unit-tested with informative console output

---

## 🧠 Sample Usage

```bash
# Compile
g++ -std=c++17 SimpleVectorDB.cpp -o vector_db

# Run
./vector_db
````

Sample output:

```
--- Initializing SimpleVectorDB ---

--- Testing addVector ---
Added 'doc_A'.
Added 'doc_B'.
...

Searching for 3 nearest neighbors to query_vec_1 (similar to A/B):
  ID: doc_B, Similarity: 0.999
  ID: doc_A, Similarity: 0.998
...
```

---

## 🧱 Project Structure

```text
.
├── SimpleVectorDB.cpp   # Main source file (DB logic + test cases)
└── README.md            # You are here!
```

---

## 🔧 Build Instructions

Ensure a C++17+ compiler:

```bash
g++ -std=c++17 SimpleVectorDB.cpp -o vector_db
```

Or with `clang++`:

```bash
clang++ -std=c++17 SimpleVectorDB.cpp -o vector_db
```

---

## 🧰 Internals

### `class SimpleVectorDB`

* Internal storage via: `std::vector<VectorEntry>`
* Fast ID lookup using: `std::map<std::string, size_t>`
* Cosine similarity calculation
* Top-K results via `std::priority_queue`
* Random 16-character ID generation using `std::mt19937`

### `struct VectorEntry`

A simple structure that stores:

* `std::string id` – Unique identifier
* `std::vector<float> data` – The actual embedding or vector

---

## 🔮 Planned Features (Coming Soon)

Here’s what’s next on the roadmap:

* 💾 **On-Disk Persistence**
  Load/save vectors using binary serialization or memory-mapped files.

* ⚙️ **Approximate Nearest Neighbor (ANN) Search**
  Speed things up with LSH, KD-Trees, or other ANN techniques.

* 🧵 **Multithreaded Search & Insertion**
  Thread-safe operations using mutexes or concurrent data structures.

* 🧠 **Custom Distance Metrics**
  Add support for Euclidean, Manhattan, etc.

* 🚀 **REST API**
  Expose endpoints using Crow or Pistache for integration with LLMs and external apps.

* 📉 **Vector Compression / Reduction**
  Use PCA or random projections for high-dim vector optimization.

* 🧩 **Vector Metadata Support**
  Allow storing extra info (tags, labels) for each vector.

* 🔍 **Advanced Filtering**
  Filter search results based on metadata constraints.

---

## 🧪 Testing Philosophy

Tests included in `main()` cover:

* Basic vector addition & retrieval
* Random ID generation
* Handling duplicate IDs
* kNN search correctness
* Exception handling for edge cases (zero vectors, empty input, k=0, etc.)

---

## 🤖 Ideal Use Cases

* Local retrieval for RAG pipelines
* Learning core vector DB concepts
* Embedding search for small projects
* Building your own FAISS-style backend in C++

---

## 📜 License

MIT — Free to use, modify, remix, or launch into orbit.

---

## 👨‍💻 Author

Built with caffeine, cosine, and C++ by [Shaunak](https://github.com/ShaunakMore)

````

---

