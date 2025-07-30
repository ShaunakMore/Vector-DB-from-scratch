#include<iostream>
#include<vector>
#include<cmath>
#include<string>
#include<stdexcept>
#include<map>
using namespace std;

struct VectorEntry{
    string id;
    vector<float>data;
};

float CosineSimilarity(const VectorEntry& a, const VectorEntry& b)
{
    // Checking if size of the two vectors are of the same dimensions
    if(a.data.size() != b.data.size()) throw runtime_error("Vector dimensions do not match");

    // Length of vector (assuming both vectors of same size for now)
    int n = a.data.size();

    // Initializing variables for dot product, square sum of a and b 
    // Declared as float instead of int to get point accuracy 
    float dot_prod = 0;
    float sqr_sum_a = 0;
    float sqr_sum_b = 0;

    // Loop through the vector to calculate dot product, and square sum
    for(int i=0 ; i< n ; i++)
    {
        dot_prod += (a.data[i] * b.data[i]);
        sqr_sum_a += (a.data[i] * a.data[i]); 
        sqr_sum_b += (b.data[i] * b.data[i]);
    }

    // Calculate magnitude using square sum
    float mag_a = sqrt(sqr_sum_a);
    float mag_b = sqrt(sqr_sum_b);

    if(mag_a == 0 || mag_b == 0) throw runtime_error("Zero length vector encountered");

    // Calculate cos similarity
    float cos_sim = (dot_prod)/(mag_a * mag_b);

    return cos_sim;
}

class SimpleVectorDB
{
    public:
        vector<VectorEntry>VectorSpace;
        map<string,size_t> id_vector_map;

    bool addVector(const string& id, const vector<float>& dataVector)
    {
       try
       { 
            // Create new instance of VectorEntry and add the data
            VectorEntry newEntry;
            newEntry.data = dataVector;
            newEntry.id = id;
            
            // Get index of new vector
            size_t new_index = VectorSpace.size();

            // Add new vector
            VectorSpace.push_back(newEntry);

            // Update vector map to map the newly added string to it's memory locaton for quick lookup
            if(!id_vector_map[id]) id_vector_map[id] = new_index;

            return true;
        } 
        catch(const char* str){
            throw runtime_error(str);
        }
    }
};

int main(){
    return 0;
}