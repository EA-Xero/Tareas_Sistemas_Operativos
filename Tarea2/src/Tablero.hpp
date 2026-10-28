#ifndef TABLERO_HPP
#define TABLERO_HPP

#include <vector>
#include <iostream>
#include <string>
#include <mutex>
#include "Configuracion.hpp"

// Estados posibles de cada posición en el tablero
enum class EstadoCasilla {
    VACIO,
    PUMA,
    PERRO
};

// Estructura que representa cada nodo/intersección del tablero
struct Casilla {
    int id;
    EstadoCasilla estado;
    std::vector<int> vecinos; // Identificadores de casillas adyacentes válidas
};

class Tablero {
private:
    std::vector<Casilla> casillas;
    mutable std::mutex mtxTablero; // Mutex para exclusión mutua en accesos y movimientos
    
    // Función auxiliar interna para conectar dos nodos en el grafo
    void agregarArista(int u, int v);

public:
    Tablero();
    ~Tablero();

    // Inicialización del grafo (3 casillas de trono + 25 de la grilla 5x5)
    void inicializarGrafo();
    void colocarPiezasIniciales(const ConfigJuego& config = ConfigJuego());

    // Métodos para inspección y depuración
    void imprimirTableroTextual() const;
    EstadoCasilla getEstado(int id) const;

    // Métodos concurrentes seguros (Thread-safe)
    std::vector<int> obtenerVecinosLibres(int idCasilla) const;
    
    // Intenta mover una pieza de origen a destino de forma atómica
    bool intentarMoverPieza(int origen, int destino, EstadoCasilla tipoPieza);
    
    // Intenta captura del Puma: salta de 'origen' sobre 'intermedio' (donde hay un Perro) a 'destino' (VACIO)
    bool intentarCapturaPuma(int origen, int intermedio, int destino);

    // Obtener la posición actual de una ficha
    int buscarPosicionPuma() const;
    std::vector<int> buscarPosicionesPerros() const;

    // Métodos de validación agregados para la Fase 3
    std::vector<std::pair<int, int>> obtenerCapturasPosiblesPuma(int posPuma) const;
    bool pumaEstaAcorralado(int posPuma) const;
};

#endif // TABLERO_HPP