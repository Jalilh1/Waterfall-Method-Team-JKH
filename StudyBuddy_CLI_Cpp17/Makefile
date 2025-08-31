
CXX := c++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -pedantic
INCLUDES := -Iinclude
SRC := $(wildcard src/*.cpp)
OBJ := $(SRC:.cpp=.o)
BIN := study_buddy
TEST_BIN := study_buddy_tests
TEST_SRC := tests/test_runner.cpp
SRC_NO_MAIN := $(filter-out src/main.cpp, $(SRC))

# Some older libstdc++ require -lstdc++fs. Uncomment if you see fs link errors.
# LDLIBS := -lstdc++fs

all: $(BIN)

$(BIN): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDLIBS)

src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

run: all
	./$(BIN)

clean:
	rm -f $(OBJ) $(BIN)

test: $(TEST_BIN)
	./$(TEST_BIN)

$(TEST_BIN): $(SRC_NO_MAIN) $(TEST_SRC)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^ $(LDLIBS)
