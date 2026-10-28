#ifndef MATCHMAKER_HPP
#define MATCHMAKER_HPP

#include <vector>
#include <semaphore.h>
#include "MesaJuego.hpp"
#include "Configuracion.hpp"

class Matchmaker {
private:
    int totalJugadores;
    int maxMesasConcurrentes;
    ConfigJuego config;
    sem_t semMesasDisponibles; // Semáforo POSIX

public:
    Matchmaker(int N, int M, const ConfigJuego& cfg);
    ~Matchmaker();

    void iniciarTorneo();
};

#endif // MATCHMAKER_HPP