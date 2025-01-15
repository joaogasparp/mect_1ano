CXX = g++
CXXFLAGS = -Wall -std=c++17

# Lista de arquivos objetos
OBJ = trab1.o histogram.o

# Nome do executável
EXEC = trab1

# Adicione as bibliotecas SFML e a linkagem da filesystem ao comando de linkagem
LDFLAGS = -lsfml-graphics -lsfml-window -lsfml-system -lstdc++fs

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CXX) $(OBJ) -o $(EXEC) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(EXEC)
	rm -f *.png

run: $(EXEC)
	./$(EXEC)	