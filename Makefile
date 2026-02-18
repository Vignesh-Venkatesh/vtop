CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2
LDFLAGS = -lncursesw

SRC_DIR = src
BUILD_DIR = build

SRC_FILES = $(SRC_DIR)/main.cpp $(SRC_DIR)/ui.cpp $(SRC_DIR)/reader.cpp
TARGET = $(BUILD_DIR)/vtop

all: $(TARGET)

$(TARGET): $(SRC_FILES)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC_FILES) $(LDFLAGS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(TARGET)

.PHONY: all run clean
