#include "Tablero.hpp"
#include <set>
#include <queue>
#include <sstream>

// Constructor
Tablero::Tablero() {
    inicializarGrafo();
    // Las piezas las coloca MesaJuego con su config (antes se colocaban y se imprimian dos veces)
}

// Destructor
Tablero::~Tablero() {
    // Por ahora no requiere limpieza manual ya que std::vector libera su memoria sola
}

// Aristas o direcciones hacia los nodos donde estaran los perros y el puma
void Tablero::agregarArista(int u, int v) {
    casillas[u].vecinos.push_back(v);
    casillas[v].vecinos.push_back(u);
}

// Creamos el grafo
void Tablero::inicializarGrafo() {
    // Total: 28 casillas (3 en el trono [Triangulo hacia arriba como en la rubrica :v ] + 25 en la matriz 5x5)
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

    //basicamente esto:
    /*
        x
       / \
      x - x

    */

    // Conexión del trono con la fila superior de la grilla (Nodos 3, 4, 5, 6, 7)
    // Segun la figura del enunciado: 1 -> esquina izq (3) y recto a 4; 2 -> recto a 6 y esquina der (7);
    // y T baja recto hasta la casilla 5 (cruza la base 1-2 sin nodo)
    agregarArista(1, 3);
    agregarArista(1, 4);
    agregarArista(2, 6);
    agregarArista(2, 7);
    agregarArista(0, 5);
    // Conecta el triangulo al cuadrado 5x5


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
        }
    }
}

void Tablero::colocarPiezasIniciales(const ConfigJuego& config) {
    // Ahora se realizan los cambios segun el archivo de configuracion del juego, quitamos las posiciones fijas si se puede
    // En caso de que no exista el archivo, se usan las posiciones por defecto

    // 1. Limpiamos estados
    for (auto& c : casillas) {
        c.estado = EstadoCasilla::VACIO;
        c.idPieza = -1;
    }
    capturasTotales = 0;

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
                casillas[pos].idPieza = perrosColocados; // el perro N queda asociado al hilo N
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
                casillas[id].idPieza = perrosColocados;
                posicionesOcupadas.insert(id);
                perrosColocados++;
            }
        }
    }

    std::cout << "[TABLERO] Disposicion inicial lista. Puma en casilla " << posPumaActual 
              << " y " << perrosColocados << " perros posicionados.\n";
}

// Getter para obtener el estado de una casilla especifica, sabe si esta vacia, ocupada por un perro o por un puma
EstadoCasilla Tablero::getEstado(int id) const {
    GuardPosix lock(mtxTablero); // un lock para la verificacion
    if (id >= 0 && id < static_cast<int>(casillas.size())) { return casillas[id].estado; }
    return EstadoCasilla::VACIO;
}

// Imprime el tablero en un formato de texto para que sea mas facil de leer
std::string Tablero::obtenerTableroComoString() const {
    GuardPosix lock(mtxTablero);

    auto sim = [this](int id) -> std::string {
        switch (casillas[id].estado) {
            case EstadoCasilla::PUMA:  return "X";
            case EstadoCasilla::PERRO: return "O";
            default:                   return "·";
        }
    };

    std::ostringstream oss;
    oss << "\n";
    oss << "                     (" << sim(0) << ")\n";
    oss << "                    ╱ │ ╲\n";
    oss << "                  ╱   │   ╲\n";
    oss << "                ╱     │     ╲\n";
    oss << "              ╱       │       ╲\n";
    oss << "            (" << sim(1) << ")───────┼───────(" << sim(2) << ")\n";
    oss << "          ╱   │       │       │   ╲\n";
    oss << "        ╱     │       │       │     ╲\n";
    oss << "  (" << sim(3)  << ")───────(" << sim(4)  << ")───────(" << sim(5)  << ")───────(" << sim(6)  << ")───────(" << sim(7)  << ")\n";
    oss << "   │        │        │        │        │\n";
    oss << "  (" << sim(8)  << ")───────(" << sim(9)  << ")───────(" << sim(10) << ")───────(" << sim(11) << ")───────(" << sim(12) << ")\n";
    oss << "   │        │        │        │        │\n";
    oss << "  (" << sim(13) << ")───────(" << sim(14) << ")───────(" << sim(15) << ")───────(" << sim(16) << ")───────(" << sim(17) << ")\n";
    oss << "   │        │        │        │        │\n";
    oss << "  (" << sim(18) << ")───────(" << sim(19) << ")───────(" << sim(20) << ")───────(" << sim(21) << ")───────(" << sim(22) << ")\n";
    oss << "   │        │        │        │        │\n";
    oss << "  (" << sim(23) << ")───────(" << sim(24) << ")───────(" << sim(25) << ")───────(" << sim(26) << ")───────(" << sim(27) << ")\n";

    return oss.str();
}

std::vector<int> Tablero::obtenerVecinosLibres(int idCasilla) const {
    GuardPosix lock(mtxTablero);
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
    GuardPosix lock(mtxTablero);

    // Validar rangos
    if (origen < 0 || origen >= static_cast<int>(casillas.size()) ||
        destino < 0 || destino >= static_cast<int>(casillas.size())) {
        return false;
    }

    // Verificar que el origen mantenga la pieza esperada y el destino siga VACIO
    // El tablero arbitra: el movimiento debe ser a una casilla adyacente
    if (adyacente(origen, destino) &&
        casillas[origen].estado == tipoPieza && casillas[destino].estado == EstadoCasilla::VACIO) {
        casillas[destino].idPieza = casillas[origen].idPieza; // la identidad viaja con la pieza
        casillas[origen].idPieza = -1;
        casillas[origen].estado = EstadoCasilla::VACIO;
        casillas[destino].estado = tipoPieza;
        return true; // Movimiento exitoso
    }

    return false; // Otra ficha gano la casilla destino primero (Race condition evitada)
}

bool Tablero::intentarCapturaPuma(int origen, int intermedio, int destino) {
    GuardPosix lock(mtxTablero);

    // El tablero arbitra: valida rangos, adyacencia y que el salto sea en LINEA RECTA
    int n = static_cast<int>(casillas.size());
    if (origen < 0 || origen >= n || intermedio < 0 || intermedio >= n || destino < 0 || destino >= n) {
        return false;
    }

    if (adyacente(origen, intermedio) && adyacente(intermedio, destino) &&
        saltoRecto(origen, intermedio, destino) &&
        casillas[origen].estado == EstadoCasilla::PUMA &&
        casillas[intermedio].estado == EstadoCasilla::PERRO &&
        casillas[destino].estado == EstadoCasilla::VACIO) {

        casillas[origen].estado = EstadoCasilla::VACIO;
        casillas[intermedio].estado = EstadoCasilla::VACIO; // El perro es capturado
        casillas[intermedio].idPieza = -1;
        casillas[destino].estado = EstadoCasilla::PUMA;
        capturasTotales++;
        return true;
    }

    return false;
}

int Tablero::buscarPosicionPuma() const {
    GuardPosix lock(mtxTablero);
    for (size_t i = 0; i < casillas.size(); ++i) {
        if (casillas[i].estado == EstadoCasilla::PUMA) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

std::vector<int> Tablero::buscarPosicionesPerros() const {
    GuardPosix lock(mtxTablero);
    std::vector<int> perros;
    for (size_t i = 0; i < casillas.size(); ++i) {
        if (casillas[i].estado == EstadoCasilla::PERRO) {
            perros.push_back(static_cast<int>(i));
        }
    }
    return perros;
}

std::vector<std::pair<int, int>> Tablero::obtenerCapturasPosiblesPuma(int posPuma) const {
    GuardPosix lock(mtxTablero);
    std::vector<std::pair<int, int>> capturas;

    if (posPuma < 0 || posPuma >= static_cast<int>(casillas.size())) return capturas;

    for (int vecinoId : casillas[posPuma].vecinos) {
        // Si hay un perro adyacente
        if (casillas[vecinoId].estado == EstadoCasilla::PERRO) {
            // Buscar si hay una casilla vacía detrás
            for (int saltoId : casillas[vecinoId].vecinos) {
                if (saltoId != posPuma && casillas[saltoId].estado == EstadoCasilla::VACIO &&
                    saltoRecto(posPuma, vecinoId, saltoId)) {
                    capturas.push_back({vecinoId, saltoId});
                }
            }
        }
    }
    return capturas;
}

bool Tablero::pumaEstaAcorralado(int posPuma) const {
    GuardPosix lock(mtxTablero);
    if (posPuma < 0 || posPuma >= static_cast<int>(casillas.size())) return false;

    // 1. Revisar si tiene movimientos simples a casillas vacías
    for (int v : casillas[posPuma].vecinos) {
        if (casillas[v].estado == EstadoCasilla::VACIO) return false;
    }

    // 2. Revisar si tiene saltos de captura posibles
    for (int v : casillas[posPuma].vecinos) {
        if (casillas[v].estado == EstadoCasilla::PERRO) {
            for (int salto : casillas[v].vecinos) {
                if (salto != posPuma && casillas[salto].estado == EstadoCasilla::VACIO &&
                    saltoRecto(posPuma, v, salto)) {
                    return false; // Puede capturar, no está acorralado
                }
            }
        }
    }

    return true; // No tiene ningún movimiento válido disponible
}

bool Tablero::esVecino(int nodoA, int nodoB) const {
    GuardPosix lock(mtxTablero);
    if (nodoA < 0 || nodoA >= (int)casillas.size()) return false;
    for (int v : casillas[nodoA].vecinos) {
        if (v == nodoB) return true;
    }
    return false;
}

// Cálculo de distancia mediante BFS simple en el grafo
int Tablero::obtenerDistanciaGrafo(int origen, int destino) const {
    GuardPosix lock(mtxTablero);
    if (origen == destino) return 0;

    std::vector<int> dist(casillas.size(), -1);
    std::queue<int> q;

    dist[origen] = 0;
    q.push(origen);

    while (!q.empty()) {
        int actual = q.front();
        q.pop();

        if (actual == destino) return dist[actual];

        for (int vecino : casillas[actual].vecinos) {
            if (dist[vecino] == -1) {
                dist[vecino] = dist[actual] + 1;
                q.push(vecino);
            }
        }
    }
    return 999; // Si no hay camino
}

// ---- Agregados (version rubrica) ----

// Auxiliar SIN lock: true si b es vecino directo de a en el grafo
bool Tablero::adyacente(int a, int b) const {
    for (int v : casillas[a].vecinos) {
        if (v == b) return true;
    }
    return false;
}

// Auxiliar SIN lock: true si el salto a -> m -> b va en linea recta.
// Dentro de la grilla (ids 3..27) basta comparar el desplazamiento (fila, columna) de ambos tramos.
// Las lineas que pasan por el trono son pocas y se listan a mano segun la figura del enunciado.
bool Tablero::saltoRecto(int a, int m, int b) const {
    if (a >= 3 && m >= 3 && b >= 3) {
        int dfA = (m - 3) / 5 - (a - 3) / 5, dcA = (m - 3) % 5 - (a - 3) % 5;
        int dfB = (b - 3) / 5 - (m - 3) / 5, dcB = (b - 3) % 5 - (m - 3) % 5;
        return dfA == dfB && dcA == dcB;
    }
    static const int lineas[5][3] = { {0, 1, 3}, {0, 2, 7}, {0, 5, 10}, {1, 4, 9}, {2, 6, 11} };
    for (const auto& l : lineas) {
        if ((a == l[0] && m == l[1] && b == l[2]) || (a == l[2] && m == l[1] && b == l[0])) return true;
    }
    return false;
}

// Posicion del perro con identidad idPerro (0..11), o -1 si ya fue capturado
int Tablero::buscarPosicionPerroPorId(int idPerro) const {
    GuardPosix lock(mtxTablero);
    for (size_t i = 0; i < casillas.size(); ++i) {
        if (casillas[i].estado == EstadoCasilla::PERRO && casillas[i].idPieza == idPerro) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

// Invariantes que el Juez comprueba: hay exactamente 1 Puma y perros en tablero + capturados = 12.
// Todo se lee bajo UN solo lock, asi que es una foto coherente del tablero.
bool Tablero::estadoConsistente() const {
    GuardPosix lock(mtxTablero);
    int pumas = 0, perros = 0;
    for (const auto& c : casillas) {
        if (c.estado == EstadoCasilla::PUMA) pumas++;
        else if (c.estado == EstadoCasilla::PERRO) perros++;
    }
    return pumas == 1 && perros + capturasTotales == 12;
}
