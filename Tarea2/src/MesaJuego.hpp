#ifndef MESA_JUEGO_HPP
#define MESA_JUEGO_HPP

#include <thread>
#include <vector>
#include <atomic>
#include <random>
#include <fstream>
#include <mutex>
#include "Tablero.hpp"
#include "Configuracion.hpp"

enum class ResultadoPartida {
    EN_PROGRESO,
    VICTORIA_PUMA,
    VICTORIA_PERROS,
    ABANDONO
};

class MesaJuego {
private:
    int idMesa;
    int idJugadorPuma;
    int idJugadorPerros;
    
    Tablero tablero;
    ConfigJuego config;
    std::atomic<bool> juegoActivo{false};
    std::atomic<int> perrosCapturados{0};
    std::atomic<int> totalMovimientos{0};
    std::atomic<ResultadoPartida> resultado{ResultadoPartida::EN_PROGRESO};

    std::ofstream archivoLog;
    std::mutex mtxLog; // Mutex para escrituras seguras en el archivo de log

    std::vector<std::thread> hilosPerros;
    std::thread hiloPuma;
    std::thread hiloJuez;

    // Rutinas ejecutadas por cada hilo
    void rutinaPuma();
    void rutinaPerro(int idPerro);
    void rutinaJuez();

    // Generador de retardo aleatorio entre delayMin y delayMax
    void esperarRetardoAleatorio(std::mt19937& rng);
    void escribirLog(const std::string& mensaje);

public:
    MesaJuego(int idMesa, int pumaPlayerId, int perrosPlayerId, const ConfigJuego& cfg);
    ~MesaJuego();

    void iniciarPartida();
    void esperarTermino();
    void abortarPorDesconexion();

    ResultadoPartida getResultado() const { return resultado.load(); }
    int getIdMesa() const { return idMesa; }
};

#endif // MESA_JUEGO_HPP