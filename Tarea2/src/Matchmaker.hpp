#ifndef MATCHMAKER_HPP
#define MATCHMAKER_HPP

#include <vector>
#include <atomic>
#include <mutex>
#include <thread>
#include <csignal>
#include <semaphore.h>
#include "MesaJuego.hpp"
#include "Configuracion.hpp"

class Matchmaker {
private:
    int totalJugadores;
    int maxMesasConcurrentes;
    ConfigJuego config;
    sem_t semMesasDisponibles; // Semaforo POSIX

    // Resiliencia ante SIGINT
    std::atomic<bool> detener{false};        // true = no aceptar mas jugadores y cerrar las partidas
    std::mutex mtxMesasActivas;              // protege mesasActivas
    std::vector<MesaJuego*> mesasActivas;    // registro de partidas en curso, para notificarlas
    sigset_t mascaraSenales;                 // contiene solo SIGINT
    std::thread hiloSenales;                 // hilo dedicado que espera SIGINT con sigwait
    std::atomic<bool> finalizandoSenales{false};

    void iniciarManejoSenales();
    void detenerManejoSenales();
    void bucleSenales();
    void notificarInterrupcion();

    void registrarMesa(MesaJuego* mesa);
    void desregistrarMesa(MesaJuego* mesa);

    // Espera un slot de mesa. Devuelve false si hay que dejar de aceptar jugadores (SIGINT o error)
    bool esperarSlot();

public:
    Matchmaker(int N, int M, const ConfigJuego& cfg);
    ~Matchmaker();

    void iniciarTorneo();
    bool fueInterrumpido() const { return detener.load(); }
};

#endif // MATCHMAKER_HPP
