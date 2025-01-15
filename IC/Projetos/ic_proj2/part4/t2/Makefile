# Compiler
CC = g++

# Compiler Flags
CFLAGS = -Wall -g -std=c++17 `pkg-config --cflags opencv4`

# Linker Flags
LDFLAGS = `pkg-config --libs opencv4`

# Source Files
SRCS = main.cpp videoCoder.cpp BitStream.cpp Golomb.cpp Predictors.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = main

# Tester Program
TESTER_SRCS = videoTester.cpp BitStream.cpp
TESTER_OBJS = $(TESTER_SRCS:.cpp=.o)
TESTER = videoTester

# Default Target
all: $(TARGET) $(TESTER)

# Encode Program Target
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Video Tester Program Target
$(TESTER): $(TESTER_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compilation Rule
%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Clean Target
clean:
	rm -f $(OBJS) $(TESTER_OBJS) $(TARGET) $(TESTER) *.bin *.mp4
