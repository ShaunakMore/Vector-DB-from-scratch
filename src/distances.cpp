#include "distances.hpp"

#include<stdexcept>
#include<cmath>

float CosineSimilarity(const std::vector<float>& a, const std::vector<float>& b)
{
    // Checking if size of the two vectors are of the same dimensions
    if(a.size() != b.size()) throw std::runtime_error("Vector dimensions do not match");

    // Length of any of the two vectors 
    int n = a.size();

    // Initializing variables for dot product, square sum of a and b 
    // Declared as float instead of int to get point accuracy 
    float dot_prod = 0;
    float sqr_sum_a = 0;
    float sqr_sum_b = 0;

    // Loop through the vector to calculate dot product, and square sum
    for(int i=0 ; i< n ; i++)
    {
        dot_prod += (a[i] * b[i]);
        sqr_sum_a += (a[i] * a[i]); 
        sqr_sum_b += (b[i] * b[i]);
    }

    // Calculate magnitude using square sum
    float mag_a = sqrt(sqr_sum_a);
    float mag_b = sqrt(sqr_sum_b);

    if(mag_a == 0 || mag_b == 0) throw std::runtime_error("Zero length vector encountered");

    // Calculate cos similarity
    float cos_sim = (dot_prod)/(mag_a * mag_b);

    return cos_sim;
}
