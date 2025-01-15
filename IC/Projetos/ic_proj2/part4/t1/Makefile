# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -O2 `pkg-config --cflags opencv4`
LDFLAGS = `pkg-config --libs opencv4`

# Source files
SRCS = main.cpp LosslessImageCodec.cpp Golomb.cpp BitStream.cpp
HDRS = LosslessImageCodec.h Golomb.h BitStream.h
OBJS = $(SRCS:.cpp=.o)

# Target executable
TARGET = main

# Build target
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp $(HDRS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up
clean:
	rm -f $(OBJS) $(TARGET)

# Phony targets
.PHONY: all clean
