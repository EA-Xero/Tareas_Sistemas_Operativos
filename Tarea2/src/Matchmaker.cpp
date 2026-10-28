#include "Matchmaker.hpp"
#include <iostream>
#include <thread>

// Archivo encargado de actuar como Juez y Emparejador de jugadores, maneja las mesas

// Constructor
Matchmaker::Matchmaker(int N, int M, const ConfigJuego& cfg)
    : totalJugadores(N), maxMesasConcurrentes(M), config(cfg) {
    sem_init(&semMesasDisponibles, 0, maxMesasConcurrentes);
}

// Destructor
Matchmaker::~Matchmaker() {
    sem_destroy(&semMesasDisponibles);
}

// Inicializa el juego una vez leido y confirmado todo
void Matchmaker::iniciarTorneo() {
    std::cout << "\n==================================================\n";
    std::cout << "[TORNEO SERVIDOR MATCHMAKER]\n";
    std::cout << "  - Jugadores a procesar (N): " << totalJugadores << "\n";
    std::cout << "  - Limite de mesas simultaneas (M): " << maxMesasConcurrentes << "\n";
    std::cout << "  - Rango de retardo: [" << config.delayMin << " ms - " << config.delayMax << " ms]\n";
    std::cout << "  - Semilla aleatoria (Seed): " << config.seed << "\n";
    std::cout << "==================================================\n";

    // Calcula total de emparejamiento posible
    int parejasTotales = totalJugadores / 2;
    std::vector<std::thread> hilosMesas;


    for (int i = 0; i < parejasTotales; ++i) {
        int idPuma = i * 2 + 1;
        int idPerros = i * 2 + 2;
        int idMesa = i + 1;

        //Espera con semaforo si se llega al limite de mesas
        std::cout << "\n[MATCHMAKER] Esperando slot para Mesa " << idMesa << "...\n";
        sem_wait(&semMesasDisponibles); // Espera pasiva

        //Si hay mesa disponible, inicializa la mesa
        hilosMesas.push_back(std::thread([this, idMesa, idPuma, idPerros]() {
            std::cout << "[MATCHMAKER] Slot otorgado. Iniciando Mesa " << idMesa << ".\n";
            MesaJuego mesa(idMesa, idPuma, idPerros, this->config);
            mesa.iniciarPartida();
            mesa.esperarTermino();
        
        // La mesa avanza hasta que termine, luego se otorga el slot a otra mesa
            std::cout << "[MATCHMAKER] Mesa " << idMesa << " finalizada. Liberando slot...\n";
            sem_post(&(this->semMesasDisponibles)); // Liberar slot
        }));
    }

    //Hace join de los hilos de mesas
    for (auto& h : hilosMesas) {
        if (h.joinable()) h.join();
    }

    // Si no hay pareja disponible para un jugador
    if (totalJugadores % 2 != 0) {
        std::cout << "\n[MATCHMAKER] Jugador " << totalJugadores << " quedo sin pareja disponible.\n";
    }

    std::cout << "\n[TORNEO] Torneo completado exitosamente.\n";
}