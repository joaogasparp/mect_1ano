# Compiler
CC = g++

# Compiler flags
CFLAGS = -Wall -std=c++17 `pkg-config --cflags opencv4`

# Linker flags
LDFLAGS = `pkg-config --libs opencv4`

# Target executable
TARGET = trab3

# Source files
SRCS = trab3.cpp ImageProcessor.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Default target
all: $(TARGET)

# Compile the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

# Compile the object files
%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up
clean:
	rm -f $(OBJS) $(TARGET)
	rm -f *.png

run: $(TARGET)
	./$(TARGET)
	