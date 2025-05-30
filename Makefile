CXX = g++
CXXFLAGS = -std=c++17 -Wall -Iinclude
SRC = $(wildcard src/*.cpp)
BIN = bin/slow-peripheral
TESTS = tests/test_connection

all: $(BIN)

$(BIN): $(SRC)
	mkdir -p bin
	$(CXX) $(CXXFLAGS) $(SRC) -o $(BIN)

$(TESTS):
	$(CXX) $(CXXFLAGS) tests/test_connection.cpp src/Connection.cpp src/Serializer.cpp -o $(TESTS)

clean:
	rm -rf bin/ tests/test_connection

.PHONY: all clean