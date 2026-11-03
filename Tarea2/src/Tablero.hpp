#ifndef TABLERO_HPP
#define TABLERO_HPP

#include <vector>
#include <iostream>
#include <string>
#include <mutex>
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
    std::vector<int> vecinos; // Identificadores de casillas adyacentes validas
};

class Tablero {
private:
    std::vector<Casilla> casillas;
    mutable std::mutex mtxTablero; // Mutex para exclusion mutua en accesos y movimientos
    
    // Funcion auxiliar interna para conectar dos nodos en el grafo
    void agregarArista(int u, int v);

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
};

#endif // TABLERO_HPP