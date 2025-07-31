#include<iostream>
#include<vector>
#include<cmath>
#include<string>
#include<stdexcept>
#include<map>
#include<random>
#include<chrono>
using namespace std;

// Custom struct to store vector and vector id
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
        // A storage to store all the VectorEntry objects and the vector id to size_t map
        vector<VectorEntry>VectorSpace;
        map<string,size_t> id_vector_map;

        const string characterString = "1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
        const size_t id_length = 16;

        // Random number generation members
        std::mt19937 generator; // The random engine
        std::uniform_int_distribution<int> random_index_gen; // The distribution

        

    SimpleVectorDB():
        // Seed the generator ONCE when the SimpleVectorDB object is created
        generator(chrono::high_resolution_clock::now().time_since_epoch().count()),

        // Initialize the distribution with the range of your characters
        random_index_gen(0, characterString.length() - 1)
    {}
    
    // Function to generate a random ID to assign to new VectorEntry types
    string randomIDGenerator()
    {
        string id = "";
        while(true)
        {
            id = "";
            id.reserve(id_length);

            for(size_t i=0; i < id_length; ++i)   id +=  characterString[random_index_gen(generator)];

            if(id_vector_map.find(id) == id_vector_map.end()) break;
            
        }
        
        

        return id;
    }

    bool addVector(const string& id, const vector<float>& dataVector)
    { 
        if(id_vector_map.find(id) != id_vector_map.end()) throw invalid_argument("Vector with id" + id + "already exists");

        // Create new instance of VectorEntry and add the data
        VectorEntry newEntry;
        newEntry.data = dataVector;
        newEntry.id = id;
        
        // Get index of new vector
        size_t new_index = VectorSpace.size();

        // Add new vector
        VectorSpace.push_back(newEntry);

        // Update vector map to map the newly added string to it's memory location for quick lookup
        id_vector_map[id] = new_index;

        return true;
    }
};

int main()
{
    SimpleVectorDB obj;
    string temp = obj.randomIDGenerator();
    cout<<temp;
    return 0;
}