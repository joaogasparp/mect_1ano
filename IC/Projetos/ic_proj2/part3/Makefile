CXX = g++
CXXFLAGS = -Wall -std=c++17

PART1_DIR = ../part1
PART2_DIR = ../part2

TARGETS = AudioCodec GolombEncoder GolombDecoder

all: $(TARGETS)

GolombEncoder: $(PART2_DIR)/GolombEncoder.cpp $(PART1_DIR)/BitStream.h $(PART2_DIR)/Golomb.h
	$(CXX) $(CXXFLAGS) -o GolombEncoder $(PART2_DIR)/GolombEncoder.cpp -lsndfile -I$(PART1_DIR) -I$(PART2_DIR)

GolombDecoder: $(PART2_DIR)/GolombDecoder.cpp $(PART1_DIR)/BitStream.h $(PART2_DIR)/Golomb.h
	$(CXX) $(CXXFLAGS) -o GolombDecoder $(PART2_DIR)/GolombDecoder.cpp -lsndfile -I$(PART1_DIR) -I$(PART2_DIR)

AudioCodec: AudioCodec.cpp $(PART1_DIR)/BitStream.h $(PART2_DIR)/Golomb.h
	$(CXX) $(CXXFLAGS) -o AudioCodec AudioCodec.cpp -lsndfile -I$(PART1_DIR) -I$(PART2_DIR)

run: $(TARGETS)
	./AudioCodec

clean:
	rm -f $(TARGETS) *.bin *.txt *.o output.wav
