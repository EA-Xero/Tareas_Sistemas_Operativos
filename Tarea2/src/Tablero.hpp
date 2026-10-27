#ifndef TABLERO_HPP
#define TABLERO_HPP

#include <vector>
#include <iostream>
#include <string>

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
};

#endif // TABLERO_HPP