CXX = g++
CXXFLAGS = -Wall -std=c++17

TARGETS = testBitStream
OBJECTS = testBitStream.o

all: $(TARGETS)

testBitStream: testBitStream.o
	$(CXX) $(CXXFLAGS) testBitStream.o -o testBitStream

testBitStream.o: testBitStream.cpp
	$(CXX) $(CXXFLAGS) -c testBitStream.cpp -o testBitStream.o

run: testBitStream
	./testBitStream

clean:
	rm -f $(OBJECTS) $(TARGETS) output.bin
