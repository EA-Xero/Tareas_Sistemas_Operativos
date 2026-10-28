#ifndef MESA_JUEGO_HPP
#define MESA_JUEGO_HPP

#include <thread>
#include <vector>
#include <atomic>
#include <random>
#include "Tablero.hpp"
#include "Configuracion.hpp"

class MesaJuego {
private:
    Tablero tablero;
    ConfigJuego config;
    std::atomic<bool> juegoActivo{true};
    std::atomic<int> perrosCapturados{0};

    std::vector<std::thread> hilosPerros;
    std::thread hiloPuma;

    // Rutinas ejecutadas por cada hilo
    void rutinaPuma();
    void rutinaPerro(int idPerro);

    // Generador de retardo aleatorio entre delayMin y delayMax[cite: 2]
    void esperarRetardoAleatorio(std::mt19937& rng);

public:
    MesaJuego(const ConfigJuego& cfg);
    ~MesaJuego();

    void iniciarPartidaPrueba(int duracionSegundos = 3);
    void detenerPartida();
};

#endif // MESA_JUEGO_HPP