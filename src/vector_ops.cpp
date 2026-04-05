#include "vector_ops.hpp"

void addInplace(std::vector<float> &a, std::vector<float> &b) {
  for (size_t i = 0; i < a.size(); i++) {
    a[i] += b[i];
  }
}

void scalarProd(const float a, std::vector<float> &vec) {
  for (float &i : vec) {
    i *= a;
  }
}