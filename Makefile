CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2 -Iinclude
LDFLAGS := -lsqlite3

SRC_DIR := src
INC_DIR := include
BUILD_DIR := build
TEST_DIR := tests

SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(filter-out $(SRC_DIR)/main.cpp, $(SRCS)))
MAIN_OBJ := $(BUILD_DIR)/main.o

TARGET := library
TEST_TARGET := run_tests

.PHONY: all clean test dirs

all: dirs $(TARGET)

dirs:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p database

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET): $(OBJS) $(MAIN_OBJ)
	$(CXX) $(OBJS) $(MAIN_OBJ) -o $(TARGET) $(LDFLAGS)

test: dirs $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): $(OBJS) $(TEST_DIR)/test_main.cpp
	$(CXX) $(CXXFLAGS) $(OBJS) $(TEST_DIR)/test_main.cpp -o $(TEST_TARGET) $(LDFLAGS)

clean:
	rm -rf $(BUILD_DIR) $(TARGET) $(TEST_TARGET) database/test_library.db

re: clean all
