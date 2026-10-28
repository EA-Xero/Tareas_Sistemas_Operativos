CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -Iinclude -pthread

# 1. Busca automáticamente todos los .cpp en src/
SRC = $(wildcard src/*.cpp)

# 2. Reemplaza la ruta src/*.cpp por src/*.o para los objetos
OBJ = $(patsubst src/%.cpp, src/%.o, $(SRC))

OUT_DIR = Juego
TARGET = $(OUT_DIR)/torneo

all: $(OUT_DIR) $(TARGET)

# Crear directorio de salida
$(OUT_DIR):
	mkdir -p $(OUT_DIR)

# Enlace de objetos para crear el ejecutable
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJ)

# Regla de compilación para convertir cada .cpp a .o
src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

cleanSrc:
	@echo "Limpiando carpeta src/"
	rm -f src/*.o

cleanLogs:
	@echo "Limpiando carpeta Logs/"
	rm -rf Logs

cleanGame:
	@echo "Limpiando carpeta Juego/"
	rm -rf $(OUT_DIR)

clean: cleanGame cleanSrc cleanLogs

.PHONY: all cleanSrc cleanLogs cleanGame clean