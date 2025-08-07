CXX = g++
CXXFLAGS = -std=c++17 -Wall -Iinclude
SRC = src/shore_db.cpp
TEST = tests/test.cpp
OUT = test

all:
	$(CXX) $(CXXFLAGS) $(SRC) $(TEST) -o $(OUT)

clean:
	rm -f $(OUT)
