#include "../include/shore_db.hpp"

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

    ShoreDB::ShoreDB():
        // Seed the generator ONCE when the ShoreDB object is created
        generator(std::chrono::high_resolution_clock::now().time_since_epoch().count()),

        // Initialize the distribution with the range of your characters
        random_index_gen(0, characterString.length() - 1)
    {}
    
    // Function to generate a random ID to assign to new VectorEntry types
    std::string ShoreDB::randomIDGenerator()
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
    bool ShoreDB::addVector(const std::string& id, const std::vector<float>& dataVector)
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
    const VectorEntry* ShoreDB::getVectorByID(const std::string& findID) const
    {
        // Search map for vector
        auto it = id_vector_map.find(findID);

        if(it == id_vector_map.end()) throw std::invalid_argument("Vector with id '"+ findID +"' does not exist");

        size_t toReturn = it->second;

        return &VectorSpace[toReturn];
    }

    // Top K nearest neighbours search function
    std::vector<std::pair<std::string,float>> ShoreDB::kNearestNeighbours(const std::vector<float>& query_vector,const int k)
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
            }catch(std::runtime_error& e){
                std::cerr<< "Warning: Skipping vector with id: '" << VectorSpace[i].id << "' because of cosine similarity error "<< e.what()<<std::endl;
                continue;
            }

            // If heap size is less than k push into heap, else if the top element
            // is smaller than the new onw pop it and add the newest one
            if(top_k.size() < static_cast<size_t>(k)) top_k.push(std::make_pair(distance,VectorSpace[i].id));

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

    // Function to save database to a binary file
    bool ShoreDB::saveToDisk(const std::string& path) const
    {
        // Create a ofstream object
        std::ofstream vecFile;
        vecFile.open(path,std::ios::binary);

        // Check if file is open
        if(!vecFile.is_open()) throw std::runtime_error("Could'nt create file");

        size_t vectorspaceSize = VectorSpace.size();

        // Binary file uses write method
        // Write the size of Vectorspace where all the vectors are stored
        vecFile.write(reinterpret_cast<const char*>(&vectorspaceSize), sizeof(size_t));

        // Iterate through each vector in vectorspace
        for(size_t i = 0; i< VectorSpace.size(); ++i)
        {
            const auto& entry = VectorSpace[i];

            // Write the size of id of vector and then the id
            size_t idLen= entry.id.size();
            vecFile.write(reinterpret_cast<const char*>(&idLen),sizeof(size_t));
            vecFile.write(reinterpret_cast<const char*>(entry.id.data()) ,idLen);

            const auto& entryVec = entry.data;

            // Write size of data and then the data
            size_t entryVecLen = entryVec.size();
            vecFile.write(reinterpret_cast<const char*>(&entryVecLen) ,sizeof(size_t));
            vecFile.write(reinterpret_cast<const char*>(entryVec.data()) ,entryVecLen * sizeof(float));
            
        }
        
        // Close file and return true
        vecFile.close();

        return true;   
    }

    bool ShoreDB::loadFromFile(const std::string& filename)
    {
        // Create ifstream object
        std::ifstream database;

        // Open database binary file
        database.open(filename,std::ios::binary);

        if(!database.is_open()) throw std::runtime_error("Could not open file to read");

        // Clear existing data
        VectorSpace.clear();
        id_vector_map.clear();

        // read size of Vector Space
        size_t vectorSpaceSize;

        database.read(reinterpret_cast<char*>(&vectorSpaceSize),sizeof(size_t));

        if(database.fail()) throw std::runtime_error("File corrupted: Failed to read Vector Space length");

        // Read all the vectors based on vector space size
        for( size_t i = 0 ; i<vectorSpaceSize ; ++i )
        {
            // Read size of id 
            size_t idLen;
            database.read(reinterpret_cast<char*>(&idLen),sizeof(size_t));

            if(database.fail()) throw std::runtime_error("File corrupted: Failed to read ID length");

            // Read id string
            std::string tempid;
            tempid.resize(idLen);
            database.read(reinterpret_cast<char*>(tempid.data()),idLen);

            if(database.fail()) throw std::runtime_error("File corrupted: Failed to read ID");

            // Read size of data
            size_t dataSize;
            database.read(reinterpret_cast<char*>(&dataSize),sizeof(size_t));

            if(database.fail()) throw std::runtime_error("File corrupted: Failed to read data length");

            // Read the data vector
            std::vector<float>tempdata(dataSize);
            database.read(reinterpret_cast<char*>(tempdata.data()), dataSize * sizeof(float));

            if(database.fail()) throw std::runtime_error("File corrupted: Failed to read data");

            // Create temporary VectorEntry instance
            VectorEntry temp;
            temp.id = tempid;
            temp.data = tempdata;

            size_t vecSize = VectorSpace.size();

            // Rebuild id vector map
            id_vector_map[tempid] = vecSize;

            // Push to Vector Space
            VectorSpace.push_back(temp);
        }

        database.close();

        return true;
    }

