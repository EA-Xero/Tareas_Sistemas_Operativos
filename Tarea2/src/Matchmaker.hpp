#ifndef MATCHMAKER_HPP
#define MATCHMAKER_HPP

#include <vector>
#include <atomic>
#include <queue>
#include <thread>
#include <csignal>
#include <semaphore.h>
#include "Sync.hpp"
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
    MutexPosix mtxMesasActivas;              // protege mesasActivas
    std::vector<MesaJuego*> mesasActivas;    // registro de partidas en curso, para notificarlas
    sigset_t mascaraSenales;                 // contiene solo SIGINT
    std::thread hiloSenales;                 // hilo dedicado que espera SIGINT con sigwait
    std::atomic<bool> finalizandoSenales{false};

    // Llegada concurrente de clientes: cada jugador es un hilo que se encola al llegar
    sem_t semJugadores;                      // cuenta los jugadores esperando en la cola
    MutexPosix mtxCola;                      // protege colaJugadores
    std::queue<int> colaJugadores;
    std::vector<std::thread> hilosClientes;
    void llegadaJugador(int id);             // rutina de cada cliente
    bool esperarJugador(int& id);            // saca al siguiente jugador (espera pasiva); false si hay SIGINT

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
