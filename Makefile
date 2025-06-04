CXX = g++
CXXFLAGS = -std=c++17 -Wall -Iinclude
SRC = $(wildcard src/*.cpp)
SRC_NO_MAIN := $(filter-out src/main.cpp, $(SRC))
BIN = bin/slow-peripheral

TEST_SRCS := $(wildcard tests/test_*.cpp)
TEST_BINS := $(patsubst tests/%.cpp, tests/%, $(TEST_SRCS))

all: $(BIN)

$(BIN): $(SRC)
	mkdir -p bin
	$(CXX) $(CXXFLAGS) $(SRC) -o $(BIN)

tests: $(TEST_BINS)
	@for test in $(TEST_BINS); do echo "Executando $$test..."; ./$$test; done

tests/%: tests/%.cpp $(SRC_NO_MAIN)
	$(CXX) $(CXXFLAGS) $^ -o $@

clean:
	rm -rf bin/
	rm -f tests/test_*[!c][!p][!p]

.PHONY: all tests clean
