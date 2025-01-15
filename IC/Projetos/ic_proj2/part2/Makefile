CXX = g++
CXXFLAGS = -Wall -std=c++17

BITSTREAM_DIR = ../part1

TARGETS = GolombEncoder GolombDecoder testGolomb
OBJECTS = GolombEncoder.o GolombDecoder.o testGolomb.o

all: $(TARGETS)

testGolomb: testGolomb.o
	$(CXX) $(CXXFLAGS) -o testGolomb testGolomb.o

testGolomb.o: testGolomb.cpp $(BITSTREAM_DIR)/BitStream.h $(BITSTREAM_DIR)/BitStream.h
	$(CXX) $(CXXFLAGS) -c testGolomb.cpp -o testGolomb.o -I$(BITSTREAM_DIR)

run: testGolomb
	./testGolomb

clean:
	rm -f $(TARGETS) *.o *.bin output.wav
