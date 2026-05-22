CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -pedantic
TARGET := assignment4

.PHONY: all clean test

all: $(TARGET)

$(TARGET): main.cpp
	$(CXX) $(CXXFLAGS) main.cpp -o $(TARGET)

test: $(TARGET)
	./run_tests.sh

clean:
	rm -f $(TARGET)
