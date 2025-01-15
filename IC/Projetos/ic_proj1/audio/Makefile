CXX = g++

CXXFLAGS = -Wall -std=c++17

SRC = audiodata_manipulation.cpp

TARGET = audiodata_manipulation

LIBS = -lsfml-audio -lsfml-system -lsfml-window -lsfml-graphics

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC) $(LIBS)

clean:
	rm -f $(TARGET)
	rm -f *.png
	rm -f audio_quantized/quantized_*

run: $(TARGET)
	./$(TARGET)
