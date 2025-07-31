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

// Custom struct to store vector and vector id
struct VectorEntry{
    std::string id;
    std::vector<float>data;
};

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

class SimpleVectorDB
{
    public:
        // A storage to store all the VectorEntry objects and the vector id to size_t map
        std::vector<VectorEntry>VectorSpace;
        std::map<std::string,size_t> id_vector_map;

        const std::string characterString = "1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
        const size_t id_length = 16;

        // Random number generation members
        std::mt19937 generator; // The random engine
        std::uniform_int_distribution<int> random_index_gen; // The distribution

    SimpleVectorDB():
        // Seed the generator ONCE when the SimpleVectorDB object is created
        generator(std::chrono::high_resolution_clock::now().time_since_epoch().count()),

        // Initialize the distribution with the range of your characters
        random_index_gen(0, characterString.length() - 1)
    {}
    
    // Function to generate a random ID to assign to new VectorEntry types
    std::string randomIDGenerator()
    {
        std::string id = "";
        while(true)
        {
            id = "";
            id.reserve(id_length);

            for(size_t i=0; i < id_length; ++i)   id +=  characterString[random_index_gen(generator)];

            if(id_vector_map.find(id) == id_vector_map.end()) break;
            
        }
        return id;
    }

    // Function to add vectors to the vector space(vector to store all Vector Entry)
    bool addVector(const std::string& id, const std::vector<float>& dataVector)
    { 
        // Check to see if a vector of the similar id already exists in the map
        if(id_vector_map.find(id) != id_vector_map.end()) throw std::invalid_argument("Vector with id '" + id + "' already exists");

        // Create new instance of VectorEntry and add the data
        VectorEntry newEntry;
        newEntry.data = dataVector;
        newEntry.id = id;
        
        /*
        Get index of new vector
        Note: cannot add pointer of the new Vector Entry to the map 
        because vector reallocates memory after every dynamic expansion
        */ 
        size_t new_index = VectorSpace.size();

        // Add new vector
        VectorSpace.push_back(newEntry);

        // Update vector map to map the newly added string to it's memory location (index) for quick lookup
        id_vector_map[id] = new_index;

        return true;
    }

    // fetch vectors by their id from the map
    const VectorEntry* getVectorByID(const std::string& findID) const
    {
        // Search map for vector
        auto it = id_vector_map.find(findID);

        if(it == id_vector_map.end()) throw std::invalid_argument("Vector with id '"+ findID +"' does not exist");

        size_t toReturn = it->second;

        return &VectorSpace[toReturn];
    }

    // Top K nearest neighbours search function
    std::vector<std::pair<std::string,float>> kNearestNeighbours(const std::vector<float>& query_vector,const int k)
    {
        // Check for argument errors
        if(query_vector.size() == 0) throw std::invalid_argument("Query vector should not be zero");

        if(k <= 0) throw std::invalid_argument("K should be greater than 0");

        if(VectorSpace.empty()) throw std::logic_error("Vector space is empty");

        // Get vector space size
        size_t vectorSpaceSize = VectorSpace.size();
        std::priority_queue<std::pair<float,std::string>, std::vector<std::pair<float,std::string>>, std::greater<std::pair<float,std::string>>> top_k;
        
        // Iterate through vector space to find cosine similarity with all the vectors
        for( size_t i = 0 ; i < vectorSpaceSize ; ++i )
        {
            float distance = 0;

            try{
                distance = CosineSimilarity(query_vector,VectorSpace[i].data);
            }catch(std::runtime_error e){
                std::cerr<< "Warning: Skipping vector with id: '" << VectorSpace[i].id << "' because of cosine similarity error "<< e.what()<<std::endl;
                continue;
            }

            // If heap size is less than k push into heap, else if the top element
            // is smaller than the new onw pop it and add the newest one
            if(top_k.size() < k) top_k.push(std::make_pair(distance,VectorSpace[i].id));

            else if(distance >  top_k.top().first){
                top_k.pop();
                top_k.push(std::make_pair(distance,VectorSpace[i].id));
            }
        }

        std::vector<std::pair<std::string,float>> topKResult;

        // Get top k similar vectors from the min heap and return the result
        while(!top_k.empty())
        {
            auto temp = top_k.top();
            topKResult.push_back(std::make_pair(temp.second,temp.first));
            top_k.pop();
        }

        std::reverse(topKResult.begin(),topKResult.end());

        return topKResult;
    }
};

int main() {
    std::cout << "--- Initializing SimpleVectorDB ---" << std::endl;
    SimpleVectorDB db;

    // --- Test addVector Functionality ---
    std::cout << "\n--- Testing addVector ---" << std::endl;
    try {
        db.addVector("doc_A", {1.0f, 1.0f, 0.0f, 0.0f});
        std::cout << "Added 'doc_A'." << std::endl;
        db.addVector("doc_B", {0.9f, 0.9f, 0.1f, 0.1f}); // Similar to A
        std::cout << "Added 'doc_B'." << std::endl;
        db.addVector("doc_C", {0.0f, 0.0f, 1.0f, 1.0f}); // Dissimilar to A/B
        std::cout << "Added 'doc_C'." << std::endl;
        db.addVector("doc_D", {0.1f, 0.1f, 0.9f, 0.9f}); // Similar to C
        std::cout << "Added 'doc_D'." << std::endl;

        // Test random ID generation and adding
        std::string rand_id_1 = db.randomIDGenerator();
        db.addVector(rand_id_1, {0.5f, 0.5f, 0.5f, 0.5f});
        std::cout << "Added random ID vector: '" << rand_id_1 << "'." << std::endl;

        std::string rand_id_2 = db.randomIDGenerator();
        db.addVector(rand_id_2, {0.2f, 0.8f, 0.2f, 0.8f});
        std::cout << "Added random ID vector: '" << rand_id_2 << "'." << std::endl;

        // Test adding a duplicate ID (should throw)
        try {
            db.addVector("doc_A", {0.1f, 0.1f, 0.1f, 0.1f});
            std::cout << "ERROR: Duplicate ID 'doc_A' was added without error!" << std::endl;
        } catch (const std::invalid_argument& e) {
            std::cout << "SUCCESS: Caught expected error for duplicate ID: " << e.what() << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "CRITICAL ERROR during addVector testing: " << e.what() << std::endl;
        return 1; // Exit with error
    }

    // --- Test getVectorByID Functionality ---
    std::cout << "\n--- Testing getVectorByID ---" << std::endl;
    try {
        const VectorEntry* vec_a = db.getVectorByID("doc_A");
        if (vec_a) {
            std::cout << "Retrieved 'doc_A': ID=" << vec_a->id << ", Data=[" << vec_a->data[0] << ", ...]" << std::endl;
        }

        // Test retrieving a non-existent ID
        try {
            db.getVectorByID("non_existent_doc");
            std::cout << "ERROR: Non-existent ID 'non_existent_doc' retrieved without error!" << std::endl;
        } catch (const std::invalid_argument& e) {
            std::cout << "SUCCESS: Caught expected error for non-existent ID: " << e.what() << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "CRITICAL ERROR during getVectorByID testing: " << e.what() << std::endl;
        return 1;
    }

    // --- Test kNearestNeighbours Functionality ---
    std::cout << "\n--- Testing kNearestNeighbours ---" << std::endl;

    // Test with an empty query vector (should throw)
    try {
        std::vector<float> empty_query = {};
        db.kNearestNeighbours(empty_query, 3);
        std::cout << "ERROR: Empty query vector did not throw error!" << std::endl;
    } catch (const std::invalid_argument& e) {
        std::cout << "SUCCESS: Caught expected error for empty query vector: " << e.what() << std::endl;
    }

    // Test with k=0 (should throw)
    try {
        std::vector<float> query = {1.0f, 0.0f, 0.0f, 0.0f};
        db.kNearestNeighbours(query, 0);
        std::cout << "ERROR: k=0 did not throw error!" << std::endl;
    } catch (const std::invalid_argument& e) {
        std::cout << "SUCCESS: Caught expected error for k=0: " << e.what() << std::endl;
    }

    // Test with an empty database (should throw - if VectorSpace was initialized empty)
    // Create a new DB instance to guarantee empty state for this test
    SimpleVectorDB empty_db;
    try {
        std::vector<float> query = {1.0f, 0.0f, 0.0f, 0.0f};
        empty_db.kNearestNeighbours(query, 1);
        std::cout << "ERROR: Empty database did not throw error!" << std::endl;
    } catch (const std::logic_error& e) { // Use logic_error as per your function
        std::cout << "SUCCESS: Caught expected error for empty database: " << e.what() << std::endl;
    }


    // Test actual search
    std::vector<float> query_vec_1 = {0.9f, 0.9f, 0.1f, 0.1f}; // Very similar to doc_A, doc_B
    std::vector<float> query_vec_2 = {0.0f, 0.0f, 0.9f, 0.9f}; // Very similar to doc_C, doc_D
    std::vector<float> query_vec_3 = {0.5f, 0.5f, 0.5f, 0.5f}; // Similar to random ID 1

    std::cout << "\nSearching for 3 nearest neighbors to query_vec_1 (similar to A/B):" << std::endl;
    try {
        auto results_1 = db.kNearestNeighbours(query_vec_1, 3);
        for (const auto& p : results_1) {
            std::cout << "  ID: " << p.first << ", Similarity: " << p.second << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error during search: " << e.what() << std::endl;
    }

    std::cout << "\nSearching for 2 nearest neighbors to query_vec_2 (similar to C/D):" << std::endl;
    try {
        auto results_2 = db.kNearestNeighbours(query_vec_2, 2);
        for (const auto& p : results_2) {
            std::cout << "  ID: " << p.first << ", Similarity: " << p.second << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error during search: " << e.what() << std::endl;
    }

    std::cout << "\nSearching for 5 nearest neighbors to query_vec_3 (general):" << std::endl;
    try {
        auto results_3 = db.kNearestNeighbours(query_vec_3, 5); // k larger than some examples
        for (const auto& p : results_3) {
            std::cout << "  ID: " << p.first << ", Similarity: " << p.second << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error during search: " << e.what() << std::endl;
    }

    // Test CosineSimilarity error propagation (e.g., zero vector in DB)
    std::cout << "\n--- Testing CosineSimilarity internal errors ---" << std::endl;
    try {
        std::string zero_vec_id = "zero_vec";
        db.addVector(zero_vec_id, {0.0f, 0.0f, 0.0f, 0.0f}); // Add a zero vector
        std::cout << "Added a zero vector with ID: '" << zero_vec_id << "'." << std::endl;

        std::vector<float> query_for_zero_test = {1.0f, 1.0f, 1.0f, 1.0f};
        std::cout << "Searching for neighbors with a zero vector in DB (expect warning):" << std::endl;
        auto results_zero_test = db.kNearestNeighbours(query_for_zero_test, 3);
        for (const auto& p : results_zero_test) {
            std::cout << "  ID: " << p.first << ", Similarity: " << p.second << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error during zero vector test: " << e.what() << std::endl;
    }


    std::cout << "\n--- All tests completed ---" << std::endl;
    return 0;
}