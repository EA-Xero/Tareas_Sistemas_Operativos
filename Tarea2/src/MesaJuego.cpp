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
                escribirLog("ESTADO DEL TABLERO TRAS LA CAPTURA:" + tablero.obtenerTableroComoString());
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
                escribirLog("ESTADO DEL TABLERO TRAS EL MOVIMIENTO:" + tablero.obtenerTableroComoString());
            }
        }
    }
}

// Movimientos y acciones de perros
void MesaJuego::rutinaPerro(int idPerro) {
    std::mt19937 rng(config.seed + idMesa * 100 + idPerro + 1);

    while (juegoActivo) {
        esperarRetardoAleatorio(rng);
        if (!juegoActivo) break;

        // 1. Obtener todas las posiciones actuales de los perros
        auto posiciones = tablero.buscarPosicionesPerros();
        if (posiciones.empty()) continue;

        // Determinar la posición de este perro específico
        int posActual = posiciones[idPerro % posiciones.size()];
        auto vecinos = tablero.obtenerVecinosLibres(posActual);

        if (vecinos.empty()) continue;

        // 2. Localizar la posición actual del Puma para calcular la estrategia
        int posPuma = tablero.buscarPosicionPuma();
        int destinoElegido = -1;
        std::string razon = "";

        if (posPuma != -1) {
            // Estrategia A: Si algún vecino está al lado del Puma, priorizarlo para acorralarlo
            for (int v : vecinos) {
                if (tablero.esVecino(v, posPuma)) {
                    destinoElegido = v;
                    razon = "Acorralamiento directo (alcanza casilla adyacente al Puma)";
                    break;
                }
            }

            // Estrategia B: Si no se puede acorralar directamente, elegir el vecino más cercano al Puma
            if (destinoElegido == -1) {
                int minDistancia = 999;
                for (int v : vecinos) {
                    int dist = tablero.obtenerDistanciaGrafo(v, posPuma);
                    if (dist < minDistancia) {
                        minDistancia = dist;
                        destinoElegido = v;
                        razon = "Reducción de distancia hacia el Puma (Distancia objetivo: " + std::to_string(dist) + ")";
                    }
                }
            }
        }

        // Fallback: Si no hay estrategia clara o no se encontró al Puma, elegir al azar
        if (destinoElegido == -1) {
            destinoElegido = vecinos[rng() % vecinos.size()];
            razon = "Movimiento exploratorio aleatorio (sin ruta directa identificada)";
        }

        // 3. Intentar el movimiento thread-safe sobre el tablero
        if (tablero.intentarMoverPieza(posActual, destinoElegido, EstadoCasilla::PERRO)) {
            totalMovimientos++;
            escribirLog("[PERRO " + std::to_string(idPerro) + "] Movimiento de " + 
                       std::to_string(posActual) + " -> " + std::to_string(destinoElegido) + 
                       " | Razón: " + razon);
            escribirLog("ESTADO DEL TABLERO TRAS EL MOVIMIENTO:" + tablero.obtenerTableroComoString());
        }
    }
}

// Encargado de determinar la partida y controlarla
void MesaJuego::rutinaJuez() {
    // Registro inicial en consola (Matchmaker)
    std::cout << "[MESA " << idMesa << "] Comienza la partida entre Jugador " 
              << idJugadorPuma << " (Puma) y Jugador " << idJugadorPerros << " (Perros)." << std::endl;

    // Registro inicial detallado en Log de la mesa
    escribirLog("=== INICIO DE LA PARTIDA EN MESA " + std::to_string(idMesa) + " ===");
    escribirLog("Jugador Puma: " + std::to_string(idJugadorPuma) + " | Jugador Perros: " + std::to_string(idJugadorPerros));
    escribirLog("ESTADO INICIAL DEL TABLERO:" + tablero.obtenerTableroComoString());

    int capturasPrevias = 0;

    while (juegoActivo) {
        // a mimir un ratico
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // Al despertar, revisa si ocurrió una nueva captura para registrarla en el Log de la mesa
        if (perrosCapturados.load() > capturasPrevias) {
            capturasPrevias = perrosCapturados.load();
            escribirLog("[JUEZ] ¡EVENTO: Captura realizada! Perros restantes: " + std::to_string(12 - capturasPrevias));
        }

        // 1. Condición de Victoria Puma (6 perros capturados)
        if (perrosCapturados.load() >= 6) {
            resultado = ResultadoPartida::VICTORIA_PUMA;
            juegoActivo = false;

            // Notificación global a consola
            std::cout << "[MESA " << idMesa << "] ¡VICTORIA DEL PUMA (Jugador " << idJugadorPuma 
                      << ")! Capturas: 6 | Movimientos totales: " << totalMovimientos.load() << std::endl;

            // Registro en Log
            escribirLog("\n=== FIN DE LA PARTIDA ===");
            escribirLog("GANADOR: PUMA (Jugador " + std::to_string(idJugadorPuma) + ")");
            escribirLog("Motivo: Se capturaron 6 perros.");
            escribirLog("Movimientos totales registrados: " + std::to_string(totalMovimientos.load()));
            escribirLog("ESTADO FINAL DEL TABLERO:" + tablero.obtenerTableroComoString());
            break;
        }

        // 2. Condición de Victoria Perros (Puma acorralado)
        int posPuma = tablero.buscarPosicionPuma();
        if (posPuma != -1 && tablero.pumaEstaAcorralado(posPuma)) {
            resultado = ResultadoPartida::VICTORIA_PERROS;
            juegoActivo = false;

            // Notificación global a consola
            std::cout << "[MESA " << idMesa << "] ¡VICTORIA DE LOS PERROS (Jugador " << idJugadorPerros 
                      << ")! Motivo: Puma acorralado | Movimientos totales: " << totalMovimientos.load() << std::endl;

            // Registro en Log
            escribirLog("\n=== FIN DE LA PARTIDA ===");
            escribirLog("GANADOR: PERROS (Jugador " + std::to_string(idJugadorPerros) + ")");
            escribirLog("Motivo: El Puma fue completamente acorralado.");
            escribirLog("Movimientos totales registrados: " + std::to_string(totalMovimientos.load()));
            escribirLog("ESTADO FINAL DEL TABLERO:" + tablero.obtenerTableroComoString());
            break;
        }
    }
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