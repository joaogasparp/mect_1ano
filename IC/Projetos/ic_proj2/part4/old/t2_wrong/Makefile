CXX = g++
CXXFLAGS = -std=c++17 -Wall -O3 `pkg-config --cflags opencv4`
LDFLAGS = `pkg-config --libs opencv4`

SRCS = main.cpp LosslessImageCodec.cpp LosslessVideoCodec.cpp BitStream.cpp Golomb.cpp Predictors.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = video_codec

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
