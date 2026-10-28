#include "Tablero.hpp"
#include <set>

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

void Tablero::colocarPiezasIniciales(const ConfigJuego& config) {
    // Ahora se realizan los cambios segun el archivo de configuracion del juego, quitamos las posiciones fijas si se puede
    // En caso de que no exista el archivo, se usan las posiciones por defecto

    // 1. Limpiamos estados
    for (auto& c : casillas) {
        c.estado = EstadoCasilla::VACIO;
    }

    // 2. Validar y ubicar la posición del Puma
    int posPumaActual = config.posPuma;
    if (posPumaActual < 0 || posPumaActual >= static_cast<int>(casillas.size())) {
        std::cout << "[ADVERTENCIA] Posicion de Puma invalida (" << posPumaActual 
                  << "). Colocando en posicion por defecto (0 - Vertice Trono).\n";
        posPumaActual = 0;
    }
    casillas[posPumaActual].estado = EstadoCasilla::PUMA;

    // 3. Ubicar los 12 Perros evitando colisiones y duplicados
    std::set<int> posicionesOcupadas;
    posicionesOcupadas.insert(posPumaActual); // El Puma ya ocupa su lugar

    int perrosColocados = 0;

    // Colocar primero las posiciones de los perros especificadas en la configuracion
    for (int pos : config.posPerros) {
        if (perrosColocados >= 12) break; // Maximo 12 perros

        if (pos >= 0 && pos < static_cast<int>(casillas.size())) {
            if (posicionesOcupadas.find(pos) == posicionesOcupadas.end()) {
                casillas[pos].estado = EstadoCasilla::PERRO;
                posicionesOcupadas.insert(pos);
                perrosColocados++;
            } else {
                std::cout << "[ADVERTENCIA] Casilla " << pos << " ocupada. Ignorando perro duplicado.\n";
            }
        } else {
            std::cout << "[ADVERTENCIA] Posicion de Perro fuera de rango: " << pos << "\n";
        }
    }

    // Si faltan perros para completar los 12, los ubicamos desde el fondo del tablero hacia arriba
    if (perrosColocados < 12) {
        std::cout << "[INFO] Autocompletando " << (12 - perrosColocados) << " perros restantes...\n";
        for (int id = 27; id >= 0 && perrosColocados < 12; --id) {
            if (posicionesOcupadas.find(id) == posicionesOcupadas.end()) {
                casillas[id].estado = EstadoCasilla::PERRO;
                posicionesOcupadas.insert(id);
                perrosColocados++;
            }
        }
    }

    std::cout << "[TABLERO] Disposicion inicial lista. Puma en casilla " << posPumaActual 
              << " y " << perrosColocados << " perros posicionados.\n";
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

std::vector<int> Tablero::obtenerVecinosLibres(int idCasilla) const {
    std::lock_guard<std::mutex> lock(mtxTablero);
    std::vector<int> libres;
    if (idCasilla < 0 || idCasilla >= static_cast<int>(casillas.size())) return libres;

    for (int vecinoId : casillas[idCasilla].vecinos) {
        if (casillas[vecinoId].estado == EstadoCasilla::VACIO) {
            libres.push_back(vecinoId);
        }
    }
    return libres;
}

bool Tablero::intentarMoverPieza(int origen, int destino, EstadoCasilla tipoPieza) {
    std::lock_guard<std::mutex> lock(mtxTablero);

    // Validar rangos
    if (origen < 0 || origen >= static_cast<int>(casillas.size()) ||
        destino < 0 || destino >= static_cast<int>(casillas.size())) {
        return false;
    }

    // Verificar que el origen mantenga la pieza esperada y el destino siga VACÍO
    if (casillas[origen].estado == tipoPieza && casillas[destino].estado == EstadoCasilla::VACIO) {
        casillas[origen].estado = EstadoCasilla::VACIO;
        casillas[destino].estado = tipoPieza;
        return true; // Movimiento exitoso
    }

    return false; // Otra ficha ganó la casilla destino primero (Race condition evitada)
}

bool Tablero::intentarCapturaPuma(int origen, int intermedio, int destino) {
    std::lock_guard<std::mutex> lock(mtxTablero);

    if (casillas[origen].estado == EstadoCasilla::PUMA &&
        casillas[intermedio].estado == EstadoCasilla::PERRO &&
        casillas[destino].estado == EstadoCasilla::VACIO) {

        casillas[origen].estado = EstadoCasilla::VACIO;
        casillas[intermedio].estado = EstadoCasilla::VACIO; // El perro es capturado[cite: 2]
        casillas[destino].estado = EstadoCasilla::PUMA;
        return true;
    }

    return false;
}

int Tablero::buscarPosicionPuma() const {
    std::lock_guard<std::mutex> lock(mtxTablero);
    for (size_t i = 0; i < casillas.size(); ++i) {
        if (casillas[i].estado == EstadoCasilla::PUMA) return static_cast<int>(i);
    }
    return -1;
}

std::vector<int> Tablero::buscarPosicionesPerros() const {
    std::lock_guard<std::mutex> lock(mtxTablero);
    std::vector<int> perros;
    for (size_t i = 0; i < casillas.size(); ++i) {
        if (casillas[i].estado == EstadoCasilla::PERRO) {
            perros.push_back(static_cast<int>(i));
        }
    }
    return perros;
}