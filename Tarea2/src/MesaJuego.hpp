#ifndef MESA_JUEGO_HPP
#define MESA_JUEGO_HPP

#include <thread>
#include <vector>
#include <atomic>
#include <random>
#include <fstream>
#include <mutex>
#include <chrono>
#include "Tablero.hpp"
#include "Configuracion.hpp"

enum class ResultadoPartida {
    EN_PROGRESO,
    VICTORIA_PUMA,
    VICTORIA_PERROS,
    ABANDONO,
    INTERRUMPIDA   // Partida cerrada de forma ordenada por SIGINT (sin ganador)
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

    // Rol del jugador que fallo: 0 = nadie, 1 = Puma, 2 = Perros. Solo el primer fallo queda registrado.
    std::atomic<int> jugadorFallido{0};
    // Solicitud externa (Matchmaker / SIGINT) de cerrar la partida de forma ordenada
    std::atomic<bool> interrumpida{false};
    // Instante de construccion de la mesa, referencia para el fallo simulado por tiempo
    std::chrono::steady_clock::time_point inicioPartida = std::chrono::steady_clock::now();

    // Rutinas ejecutadas por cada hilo
    void rutinaPuma();
    void rutinaPerro(int idPerro);
    void rutinaJuez();

    // Envoltorios que atrapan cualquier excepcion de los hilos de jugador (si no, std::terminate)
    void rutinaPumaProtegida();
    void rutinaPerroProtegida(int idPerro);

    // Llamado por el Juez en cada ciclo: atiende fallos de jugador y solicitudes de interrupcion.
    // Devuelve true si la partida fue cerrada y el Juez debe terminar su bucle.
    bool verificarEventosExternos();

    // Generador de retardo aleatorio entre delayMin y delayMax
    void esperarRetardoAleatorio(std::mt19937& rng);
    void escribirLog(const std::string& mensaje);

public:
    MesaJuego(int idMesa, int pumaPlayerId, int perrosPlayerId, const ConfigJuego& cfg);
    ~MesaJuego();

    void iniciarPartida();
    void esperarTermino();
    void abortarPorDesconexion();

    // Marca que un jugador fallo (1 = Puma, 2 = Perros). No bloquea ni hace join: es seguro
    // llamarlo desde cualquier hilo, incluido el propio hilo que fallo. El Juez cierra la partida.
    void reportarFalloJugador(int rol) noexcept;
    // Pide cerrar la partida de forma ordenada (usado por SIGINT). No bloquea, seguro desde cualquier hilo.
    void solicitarDetencion() noexcept { interrumpida.store(true); }

    ResultadoPartida getResultado() const { return resultado.load(); }
    int getIdMesa() const { return idMesa; }
};

#endif // MESA_JUEGO_HPP