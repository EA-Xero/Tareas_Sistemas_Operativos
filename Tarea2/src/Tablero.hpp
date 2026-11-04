#ifndef TABLERO_HPP
#define TABLERO_HPP

#include <vector>
#include <iostream>
#include <string>
#include "Sync.hpp"
#include "Configuracion.hpp"

// Estados posibles de cada posicion en el tablero
enum class EstadoCasilla {
    VACIO,
    PUMA,
    PERRO
};

// Estructura que representa cada nodo/interseccion del tablero
struct Casilla {
    int id;
    EstadoCasilla estado;
    int idPieza = -1; // identidad fija de cada perro (0..11); -1 = vacio o Puma
    std::vector<int> vecinos; // Identificadores de casillas adyacentes validas
};

class Tablero {
private:
    std::vector<Casilla> casillas;
    mutable MutexPosix mtxTablero; // Mutex para exclusion mutua en accesos y movimientos
    
    // Funcion auxiliar interna para conectar dos nodos en el grafo
    void agregarArista(int u, int v);

    int capturasTotales = 0; // perros capturados, contado dentro del mismo lock que el movimiento
    // Auxiliares SIN lock: solo se llaman con mtxTablero ya tomado (el mutex no es recursivo)
    bool adyacente(int a, int b) const;
    bool saltoRecto(int a, int m, int b) const;

public:
    Tablero();
    ~Tablero();

    // Inicializacion del grafo (3 casillas de trono + 25 de la grilla 5x5)
    void inicializarGrafo();
    void colocarPiezasIniciales(const ConfigJuego& config = ConfigJuego());

    // Metodos para inspeccion y depuracion
    // Retorna el tablero como std::string para los Logs
    std::string obtenerTableroComoString() const;
    EstadoCasilla getEstado(int id) const;

    // Metodos concurrentes seguros (Thread-safe)
    std::vector<int> obtenerVecinosLibres(int idCasilla) const;
    
    // Intenta mover una pieza de origen a destino de forma atomica
    bool intentarMoverPieza(int origen, int destino, EstadoCasilla tipoPieza);
    
    // Intenta captura del Puma: salta de 'origen' sobre 'intermedio' (donde hay un Perro) a 'destino' (VACIO)
    bool intentarCapturaPuma(int origen, int intermedio, int destino);

    // Obtener la posicion actual de una ficha
    int buscarPosicionPuma() const;
    std::vector<int> buscarPosicionesPerros() const;

    // Metodos de validacion agregados para la Fase 3
    std::vector<std::pair<int, int>> obtenerCapturasPosiblesPuma(int posPuma) const;
    bool pumaEstaAcorralado(int posPuma) const;
    bool esVecino(int nodoA, int nodoB) const;
    int obtenerDistanciaGrafo(int origen, int destino) const;

    // Agregados: identidad fija de cada perro y chequeo de invariantes para el Juez
    int buscarPosicionPerroPorId(int idPerro) const;
    bool estadoConsistente() const;
};

#endif // TABLERO_HPP