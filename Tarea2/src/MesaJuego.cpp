#include "MesaJuego.hpp"
#include <chrono>
#include <iostream>

MesaJuego::MesaJuego(const ConfigJuego& cfg) : config(cfg) {
    tablero.colocarPiezasIniciales(config);
}

MesaJuego::~MesaJuego() {
    detenerPartida();
}

void MesaJuego::esperarRetardoAleatorio(std::mt19937& rng) {
    std::uniform_int_distribution<int> dist(config.delayMin, config.delayMax); //[cite: 2]
    int ms = dist(rng);
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void MesaJuego::rutinaPuma() {
    std::mt19937 rng(config.seed);

    while (juegoActivo) {
        esperarRetardoAleatorio(rng);

        int posActual = tablero.buscarPosicionPuma();
        if (posActual == -1) continue;

        auto vecinos = tablero.obtenerVecinosLibres(posActual);
        if (!vecinos.empty()) {
            int destino = vecinos[rng() % vecinos.size()];
            if (tablero.intentarMoverPieza(posActual, destino, EstadoCasilla::PUMA)) {
                std::cout << "[HILO PUMA] Se movio de " << posActual << " -> " << destino << "\n";
            }
        }
    }
}

void MesaJuego::rutinaPerro(int idPerro) {
    std::mt19937 rng(config.seed + idPerro + 1);

    while (juegoActivo) {
        esperarRetardoAleatorio(rng);

        auto posiciones = tablero.buscarPosicionesPerros();
        if (posiciones.empty()) continue;

        // Cada hilo de perro intenta mover la posición que encuentra correspondiente
        int posActual = posiciones[idPerro % posiciones.size()];
        auto vecinos = tablero.obtenerVecinosLibres(posActual);

        if (!vecinos.empty()) {
            int destino = vecinos[rng() % vecinos.size()];
            if (tablero.intentarMoverPieza(posActual, destino, EstadoCasilla::PERRO)) {
                std::cout << "[HILO PERRO " << idPerro << "] Se movio de " << posActual << " -> " << destino << "\n";
            }
        }
    }
}

void MesaJuego::iniciarPartidaPrueba(int duracionSegundos) {
    std::cout << "\n=== INICIANDO PARTIDA DE PRUEBA CON CONCURRENCIA EN TIEMPO REAL ===\n";
    juegoActivo = true;

    // Lanzar Hilo del Puma[cite: 2]
    hiloPuma = std::thread(&MesaJuego::rutinaPuma, this);

    // Lanzar 12 Hilos de Perros[cite: 2]
    for (int i = 0; i < 12; ++i) {
        hilosPerros.push_back(std::thread(&MesaJuego::rutinaPerro, this, i));
    }

    // Dejar correr la simulación por unos segundos para verificar concurrencia
    std::this_thread::sleep_for(std::chrono::seconds(duracionSegundos));

    detenerPartida();
    std::cout << "\n=== ESTADO DEL TABLERO TRAS MOVIMIENTOS CONCURRENTES ===\n";
    tablero.imprimirTableroTextual();
}

void MesaJuego::detenerPartida() {
    juegoActivo = false;

    if (hiloPuma.joinable()) {
        hiloPuma.join();
    }

    for (auto& h : hilosPerros) {
        if (h.joinable()) {
            h.join();
        }
    }
    hilosPerros.clear();
}