CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -Iinclude -pthread
SRC = src/main.cpp src/Tablero.cpp src/Configuracion.cpp src/MesaJuego.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = torneo

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJ)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f src/*.o $(TARGET)

.PHONY: all clean