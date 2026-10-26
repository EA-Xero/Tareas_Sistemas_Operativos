#include "Tablero.hpp"

Tablero::Tablero() {
    inicializarGrafo();
    colocarPiezasIniciales();
}

Tablero::~Tablero() {
    // Por ahora no requiere limpieza manual ya que std::vector libera su memoria sola
}

void Tablero::agregarArista(int u, int v) {
    casillas[u].vecinos.push_back(v);
    casillas[v].vecinos.push_back(u);
}

void Tablero::inicializarGrafo() {
    // Total: 28 casillas (3 en el trono + 25 en la matriz 5x5)
    casillas.resize(28);
    for (int i = 0; i < 28; ++i) {
        casillas[i].id = i;
        casillas[i].estado = EstadoCasilla::VACIO;
    }

    // 1. Conexiones del Trono (Nodos 0, 1, 2)
    // 0 = Vértice T, 1 = Izquierda del trono, 2 = Derecha del trono
    agregarArista(0, 1);
    agregarArista(0, 2);
    agregarArista(1, 2);

    // Conexión del trono con la fila superior de la grilla (Nodos 3, 4, 5, 6, 7)
    agregarArista(1, 3);
    agregarArista(1, 5);
    agregarArista(2, 5);
    agregarArista(2, 7);

    // 2. Conexiones de la Grilla 5x5 (Nodos 3 al 27)
    // Mapeo: fila = (id - 3) / 5, columna = (id - 3) % 5
    for (int r = 0; r < 5; ++r) {
        for (int c = 0; c < 5; ++c) {
            int actual = 3 + r * 5 + c;
            
            // Conexión horizontal derecha
            if (c < 4) {
                agregarArista(actual, actual + 1);
            }
            // Conexión vertical hacia abajo
            if (r < 4) {
                agregarArista(actual, actual + 5);
            }
            // Conexiones diagonales opcionales según el diseño de tablero Komikan tradicional
            if ((r + c) % 2 == 0) {
                if (r < 4 && c < 4) agregarArista(actual, actual + 6);
                if (r < 4 && c > 0) agregarArista(actual, actual + 4);
            }
        }
    }
}

void Tablero::colocarPiezasIniciales() {
    // Disposición básica inicial estándar:
    // Ponemos al Puma en el vértice del trono por defecto [0].
    casillas[0].estado = EstadoCasilla::PUMA;

    // Los 12 perros comienzan ocupando las filas inferiores (para simplicidad).
    // Asignamos casillas de la 16 a la 27
    for (int i = 16; i <= 27; ++i) {
        casillas[i].estado = EstadoCasilla::PERRO;
    }
}

// Getter para obtener el estado de una casilla específica, sabe si esta vacia, ocupada por un perro o por un puma
EstadoCasilla Tablero::getEstado(int id) const {
    if (id >= 0 && id < static_cast<int>(casillas.size())) {
        return casillas[id].estado;
    }
    return EstadoCasilla::VACIO;
}

// Imprime el tablero en un formato de texto para que sea mas facil de leer
void Tablero::imprimirTableroTextual() const {
    std::cout << "\n=== ESTADO DEL TABLERO ===\n";
    auto charEstado = [](EstadoCasilla e) -> char {
        switch (e) {
            case EstadoCasilla::PUMA: return 'X';  // Puma
            case EstadoCasilla::PERRO: return 'O'; // Perro
            default: return '.';                  // Vacío
        }
    };

    std::cout << "       [" << charEstado(casillas[0].estado) << "]\n";
    std::cout << "     [" << charEstado(casillas[1].estado) << "]   [" << charEstado(casillas[2].estado) << "]\n";
    std::cout << "  -----------------\n";

    for (int r = 0; r < 5; ++r) {
        std::cout << "  ";
        for (int c = 0; c < 5; ++c) {
            int id = 3 + r * 5 + c;
            std::cout << charEstado(casillas[id].estado) << "   ";
        }
        std::cout << "\n";
    }
    std::cout << "===========================\n\n";
}