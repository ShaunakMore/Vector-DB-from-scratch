# 🧠 ShoreDB – A Simple In-Memory Vector Database in C++

ShoreDB is a lightweight, header-only, in-memory vector database built from scratch using C++. It supports basic operations such as insertion, deletion, saving/loading to disk, and Approximate Nearest Neighbour (ANN) search using Euclidean distance.

---

## ✨ Features

* Insert vectors with string IDs
* Delete vectors by ID
* k-Nearest Neighbours (k-NN) search
* Save database to disk
* Load database from disk
* Built for extensibility and performance

---

## 📁 Project Structure

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

---

## ⚙️ Build Instructions

Make sure you have `g++` installed. Then, from the root directory:

```bash
make
```

This will compile the source files and generate an executable named `test`.

---

## 🚀 Run

After building:

```bash
./test
```

It will run the included tests and print output based on vector operations.

---

## 🧪 Example Test Scenario

The `test.cpp` file contains a few example inserts, saves to disk, loads from disk, and a nearest-neighbour search.

---

## 📝 Notes

* ShoreDB currently only uses **Euclidean distance** for similarity.
* This is a basic implementation aimed at understanding the internals of vector DBs, not optimized for production use.
* ANN is implemented using a priority queue approach (brute-force for now).

---

## 📌 TODO

* [ ] Implement cosine similarity
* [ ] Add more efficient ANN indexing (e.g., KD-Tree, HNSW)
* [ ] Add benchmarking utilities
* [ ] Add CLI interface

---

## 📖 License

MIT License – do whatever you want, just give credit. ✌️

---

## 👨‍💻 Author

Shaunak More
Built with ☕, curiosity, and late-night debugging.
