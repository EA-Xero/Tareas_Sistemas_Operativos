#include "MesaJuego.hpp"
#include <chrono>
#include <iostream>
#include <iomanip>
#include <sys/stat.h>

// Se encarga de crear los Logs para cada mesa y hacer posible su operacion
// Constructor
MesaJuego::MesaJuego(int id, int pumaId, int perrosId, const ConfigJuego& cfg) 
    : idMesa(id), idJugadorPuma(pumaId), idJugadorPerros(perrosId), config(cfg) {
    tablero.colocarPiezasIniciales(config);
    
    // 1. Crear carpeta Logs/ si no existe
    mkdir("Logs", 0755);

    // 2. Ruta dentro de la carpeta Logs/
    std::string nombreArchivo = "Logs/mesa_" + std::to_string(idMesa) + ".log";
    
    // 3. Abrir en modo std::ios::app (Añadir al final sin sobrescribir)
    archivoLog.open(nombreArchivo, std::ios::out | std::ios::app);
    if (archivoLog.is_open()) {
        archivoLog << "\n==================================================" << std::endl;
        archivoLog << "=== NUEVA PARTIDA REGISTRADA - MESA " << idMesa << " ===" << std::endl;
        archivoLog << "Jugador Puma ID: " << idJugadorPuma << " | Jugador Perros ID: " << idJugadorPerros << std::endl;
        archivoLog << "==================================================" << std::endl;
    }
}

// Destructor
MesaJuego::~MesaJuego() {
    juegoActivo = false;
    esperarTermino();
    if (archivoLog.is_open()) {
        archivoLog.close();
    }
}

// Lo dice el nombre ;V
void MesaJuego::escribirLog(const std::string& mensaje) {
    std::lock_guard<std::mutex> lock(mtxLog);
    if (archivoLog.is_open()) {
        archivoLog << mensaje << std::endl;
    }
}

// Utiliza la funcion mt19937 para un random bueno, basicamente con esa funcion nativa calculamos el retardo del jugador
void MesaJuego::esperarRetardoAleatorio(std::mt19937& rng) {
    std::uniform_int_distribution<int> dist(config.delayMin, config.delayMax);
    int ms = dist(rng);
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

// Funciones de juego para el puma
void MesaJuego::rutinaPuma() {
    // Valor aleatorio de retardo de juego para el puma
    std::mt19937 rng(config.seed + idMesa * 100);

    while (juegoActivo) {
        esperarRetardoAleatorio(rng);
        if (!juegoActivo) break;

        int posActual = tablero.buscarPosicionPuma();
        if (posActual == -1) continue;

        // Prioridad 1: Intentar Captura

        // matriz que trae valores si existe la posibilidad de capturar algo
        auto capturas = tablero.obtenerCapturasPosiblesPuma(posActual);
        // Si la matriz de capturas posibles no esta vacia, entra en el if
        if (!capturas.empty()) {
            // le asigna un destino al intento de captura, dado que realmente son dos movimientos (pasar por el perro, terminar en destino)
            auto cap = capturas[rng() % capturas.size()];
            
            // intenta la captura, exito o fracaso se manejan a nivel mas bajo, en el archivo Tablero.cpp
            if (tablero.intentarCapturaPuma(posActual, cap.first, cap.second)) {
                perrosCapturados++;
                totalMovimientos++;
                escribirLog("[PUMA] CAPTURA Exitosa: Salto desde " + std::to_string(posActual) + 
                           " sobre perro en " + std::to_string(cap.first) + 
                           " hacia " + std::to_string(cap.second) + 
                           " (Total capturados: " + std::to_string(perrosCapturados.load()) + ")");
                continue;
            }
        }

        // Prioridad 2: Movimiento simple

        //Mira si tiene algun punto libre a su alrededor, si lo tiene se mueve
        auto vecinos = tablero.obtenerVecinosLibres(posActual);
        // si la matriz es no vacia, verificamos
        if (!vecinos.empty()) {
            // buscamos un destino, asignamos un id al intento
            int destino = vecinos[rng() % vecinos.size()];
            //Mismo caso que intento de captura, se maneja en tablero.cpp
            if (tablero.intentarMoverPieza(posActual, destino, EstadoCasilla::PUMA)) {
                totalMovimientos++;
                escribirLog("[PUMA] Movimiento de " + std::to_string(posActual) + " -> " + std::to_string(destino));
            }
        }
    }
}

// Movimientos y acciones de perros
void MesaJuego::rutinaPerro(int idPerro) {
    // retardo aleatorio para el perro
    std::mt19937 rng(config.seed + idMesa * 100 + idPerro + 1);


    while (juegoActivo) {
        esperarRetardoAleatorio(rng);
        if (!juegoActivo) break;

        // busca posiciones disponibles
        auto posiciones = tablero.buscarPosicionesPerros();
        // si no hay, continuar al siguiente ciclo
        if (posiciones.empty()) continue;

        //Agarra su posicion actual
        int posActual = posiciones[idPerro % posiciones.size()];
        //Agarra posiciones de los vecinos
        auto vecinos = tablero.obtenerVecinosLibres(posActual);

        // Si la matriz de vecinos no es vacia entra
        if (!vecinos.empty()) {
            // Selecciona destino
            int destino = vecinos[rng() % vecinos.size()];
            // Intenta moverse
            if (tablero.intentarMoverPieza(posActual, destino, EstadoCasilla::PERRO)) {
                totalMovimientos++;
                escribirLog("[PERRO " + std::to_string(idPerro) + "] Movimiento de " + 
                           std::to_string(posActual) + " -> " + std::to_string(destino));
            }
        }
    }
}

// Encargado de determinar la partida y controlarla
void MesaJuego::rutinaJuez() {
    std::cout << "\n[JUEZ MESA " << idMesa << "] Comienza la partida entre Jugador " 
              << idJugadorPuma << " (Puma) y Jugador " << idJugadorPerros << " (Perros).\n";
    std::cout << "[JUEZ MESA " << idMesa << "] Estado Inicial del Tablero:";
    tablero.imprimirTableroTextual();

    int capturasPrevias = 0;

    while (juegoActivo) {
        // a mimir un ratico
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // Al despertar, revisa si ocurrió una nueva captura, el juez imprime actualización en consola
        if (perrosCapturados.load() > capturasPrevias) {
            capturasPrevias = perrosCapturados.load();
            std::cout << "[JUEZ MESA " << idMesa << "] ¡EVENTO: Captura realizada! Perros restantes: " 
                      << (12 - capturasPrevias) << "\n";
            tablero.imprimirTableroTextual();
        }

        // 1. Condición de Victoria Puma (6 perros capturados)
        if (perrosCapturados.load() >= 6) {
            resultado = ResultadoPartida::VICTORIA_PUMA;
            std::cout << "\n==============================================";
            std::cout << "\n[JUEZ MESA " << idMesa << "] ¡VICTORIA DEL PUMA (Jugador " << idJugadorPuma << ")!";
            std::cout << "\nMotivo: Se capturaron 6 perros. Movimientos totales: " << totalMovimientos.load();
            std::cout << "\n==============================================\n";
            juegoActivo = false;
            break;
        }

        // 2. Condición de Victoria Perros (Puma acorralado)
        int posPuma = tablero.buscarPosicionPuma();
        if (posPuma != -1 && tablero.pumaEstaAcorralado(posPuma)) {
            resultado = ResultadoPartida::VICTORIA_PERROS;
            std::cout << "\n==============================================";
            std::cout << "\n[JUEZ MESA " << idMesa << "] ¡VICTORIA DE LOS PERROS (Jugador " << idJugadorPerros << ")!";
            std::cout << "\nMotivo: El Puma fue completamente acorralado. Movimientos totales: " << totalMovimientos.load();
            std::cout << "\n==============================================\n";
            juegoActivo = false;
            break;
        }
    }

    std::cout << "[JUEZ MESA " << idMesa << "] Estado Final del Tablero:";
    tablero.imprimirTableroTextual();
    escribirLog("=== FIN DE LA PARTIDA. Movimientos totales registrados: " + std::to_string(totalMovimientos.load()) + " ===");
}

// Inicia la partida, con configuracion de perro, puma y id ya cargada
void MesaJuego::iniciarPartida() {
    juegoActivo = true;
    hiloPuma = std::thread(&MesaJuego::rutinaPuma, this);

    for (int i = 0; i < 12; ++i) {
        hilosPerros.push_back(std::thread(&MesaJuego::rutinaPerro, this, i));
    }

    hiloJuez = std::thread(&MesaJuego::rutinaJuez, this);
}

// Espera a que termine, junta los hilos y libera memoria
void MesaJuego::esperarTermino() {
    if (hiloJuez.joinable()) hiloJuez.join();
    if (hiloPuma.joinable()) hiloPuma.join();

    for (auto& h : hilosPerros) {
        if (h.joinable()) h.join();
    }
    hilosPerros.clear();
}

// Por si un jugador se pica, que no nos deje la caga
void MesaJuego::abortarPorDesconexion() {
    resultado = ResultadoPartida::ABANDONO;
    escribirLog("[ABORTADO] La partida finalizo abruptamente por desconexion/fallo.");
    juegoActivo = false;
    esperarTermino();
}