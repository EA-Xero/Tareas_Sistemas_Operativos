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
    GuardPosix lock(mtxLog);
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

        // 1. Cada hilo controla SIEMPRE al mismo perro (identidad fija dentro del tablero)
        int posActual = tablero.buscarPosicionPerroPorId(idPerro);
        if (posActual == -1) { // su perro fue capturado: el hilo ya no tiene pieza que mover
            escribirLog("[PERRO " + std::to_string(idPerro) + "] Capturado, su hilo termina.");
            break;
        }
        auto vecinos = tablero.obtenerVecinosLibres(posActual);

        if (vecinos.empty()) continue;

        // 2. Localizar la posicion actual del Puma para calcular la estrategia
        int posPuma = tablero.buscarPosicionPuma();
        int destinoElegido = -1;
        std::string razon = "";

        if (posPuma != -1) {
            // Estrategia A: Si algun vecino esta al lado del Puma, priorizarlo para acorralarlo
            for (int v : vecinos) {
                if (tablero.esVecino(v, posPuma)) {
                    destinoElegido = v;
                    razon = "Acorralamiento directo (alcanza casilla adyacente al Puma)";
                    break;
                }
            }

            // Estrategia B: Si no se puede acorralar directamente, elegir el vecino mas cercano al Puma
            if (destinoElegido == -1) {
                int minDistancia = 999;
                for (int v : vecinos) {
                    int dist = tablero.obtenerDistanciaGrafo(v, posPuma);
                    if (dist < minDistancia) {
                        minDistancia = dist;
                        destinoElegido = v;
                        razon = "Reduccion de distancia hacia el Puma (Distancia objetivo: " + std::to_string(dist) + ")";
                    }
                }
            }
        }

        // Fallback: Si no hay estrategia clara o no se encontro al Puma, elegir al azar
        if (destinoElegido == -1) {
            destinoElegido = vecinos[rng() % vecinos.size()];
            razon = "Movimiento exploratorio aleatorio (sin ruta directa identificada)";
        }

        // 3. Intentar el movimiento thread-safe sobre el tablero
        if (tablero.intentarMoverPieza(posActual, destinoElegido, EstadoCasilla::PERRO)) {
            totalMovimientos++;
            escribirLog("[PERRO " + std::to_string(idPerro) + "] Movimiento de " + 
                       std::to_string(posActual) + " -> " + std::to_string(destinoElegido) + 
                       " | Razon: " + razon);
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

        // abandono de un jugador o SIGINT cierran la partida
        if (verificarEventosExternos()) break;

        // El Juez verifica los invariantes del tablero en cada ciclo
        if (!tablero.estadoConsistente()) {
            escribirLog("[JUEZ] ALERTA: estado del tablero inconsistente");
        }

        // Al despertar, revisa si ocurrio una nueva captura para registrarla en el Log de la mesa
        if (perrosCapturados.load() > capturasPrevias) {
            capturasPrevias = perrosCapturados.load();
            escribirLog("[JUEZ] ¡EVENTO: Captura realizada! Perros restantes: " + std::to_string(12 - capturasPrevias));
        }

        // 1. Condicion de Victoria Puma (6 perros capturados)
        if (perrosCapturados.load() >= 6) {
            resultado = ResultadoPartida::VICTORIA_PUMA;
            juegoActivo = false;

            // Notificacion global a consola
            std::cout << "[MESA " << idMesa << "] ¡VICTORIA DEL PUMA (Jugador " << idJugadorPuma 
                      << ")! Capturas: " << perrosCapturados.load() << " | Movimientos totales: " << totalMovimientos.load() << std::endl;

            // Registro en Log
            escribirLog("\n=== FIN DE LA PARTIDA ===");
            escribirLog("GANADOR: PUMA (Jugador " + std::to_string(idJugadorPuma) + ")");
            escribirLog("Motivo: Se capturaron 6 perros.");
            escribirLog("Movimientos totales registrados: " + std::to_string(totalMovimientos.load()));
            escribirLog("ESTADO FINAL DEL TABLERO:" + tablero.obtenerTableroComoString());
            break;
        }

        // 2. Condicion de Victoria Perros (Puma acorralado)
        int posPuma = tablero.buscarPosicionPuma();
        if (posPuma != -1 && tablero.pumaEstaAcorralado(posPuma)) {
            resultado = ResultadoPartida::VICTORIA_PERROS;
            juegoActivo = false;

            // Notificacion global a consola
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
    hiloPuma = std::thread(&MesaJuego::rutinaPumaProtegida, this);

    for (int i = 0; i < 12; ++i) {
        hilosPerros.push_back(std::thread(&MesaJuego::rutinaPerroProtegida, this, i));
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

// Registra que un jugador se cayo. Solo escribe atomicos, asi que no bloquea y es seguro desde cualquier hilo.
// compare_exchange: si ya habia un fallo registrado, el primero prevalece (no se pisa el ganador)
void MesaJuego::reportarFalloJugador(int rol) noexcept {
    int esperado = 0;
    jugadorFallido.compare_exchange_strong(esperado, rol);
}

// Envoltorio del hilo Puma: una excepcion que escapa de un std::thread llama a std::terminate
// y mata todo el torneo. Aqui se convierte en "el jugador Puma fallo".
// Al propagarse la excepcion, los lock_guard (Tablero y log) liberan sus mutex por RAII.
void MesaJuego::rutinaPumaProtegida() {
    try { rutinaPuma(); } 
    catch (const std::exception& e) {
        reportarFalloJugador(1);
        try { escribirLog(std::string("[FALLO] Hilo del Puma termino con excepcion: ") + e.what()); } catch (...) {} // e.what da el string de la excepcion
    } 
    catch (...) { reportarFalloJugador(1); }
}

// Idem para cada hilo de perro: el fallo de cualquiera se atribuye al jugador Perros
void MesaJuego::rutinaPerroProtegida(int idPerro) {
    try {
        rutinaPerro(idPerro);
    } catch (const std::exception& e) {
        reportarFalloJugador(2);
        try { escribirLog("[FALLO] Hilo del perro " + std::to_string(idPerro) +
                          " termino con excepcion: " + e.what()); } catch (...) {}
    } catch (...) { reportarFalloJugador(2); }
}

// Ejecutado SOLO por el hilo Juez en cada ciclo. Al ser el unico que cierra la partida, no hay
// dos hilos escribiendo el resultado final ni haciendo join sobre los mismos hilos.
bool MesaJuego::verificarEventosExternos() {
    // Fallo simulado (config): "se cae" un jugador pasado cierto tiempo desde el inicio
    if (config.falloJugador != 0 &&
        (config.falloMesa == 0 || config.falloMesa == idMesa) &&
        jugadorFallido.load() == 0 &&
        std::chrono::steady_clock::now() - inicioPartida >= std::chrono::milliseconds(config.falloTrasMs)) {
        escribirLog("[JUEZ] Fallo simulado: se desconecta el jugador " +
                    std::string(config.falloJugador == 1 ? "Puma" : "Perros"));
        reportarFalloJugador(config.falloJugador);
    }

    // Abandono: gana el oponente del jugador que fallo
    int rol = jugadorFallido.load();
    if (rol != 0) {
        resultado = ResultadoPartida::ABANDONO;
        juegoActivo = false; // los hilos restantes salen de su bucle y el Matchmaker los une

        bool cayoPuma = (rol == 1);
        int idCaido = cayoPuma ? idJugadorPuma : idJugadorPerros;
        int idGanador = cayoPuma ? idJugadorPerros : idJugadorPuma;
        std::string rolGanador = cayoPuma ? "PERROS" : "PUMA";

        std::cout << "[MESA " << idMesa << "] ¡VICTORIA POR ABANDONO! Jugador " << idCaido
                  << " (" << (cayoPuma ? "Puma" : "Perros") << ") fallo. Gana " << rolGanador
                  << " (Jugador " << idGanador << ")." << std::endl;

        escribirLog("\n=== FIN DE LA PARTIDA (ABANDONO) ===");
        escribirLog("GANADOR: " + rolGanador + " (Jugador " + std::to_string(idGanador) + ") por abandono");
        escribirLog("Motivo: El jugador " + std::to_string(idCaido) + " fallo o se desconecto.");
        escribirLog("Movimientos totales registrados: " + std::to_string(totalMovimientos.load()));
        escribirLog("ESTADO FINAL DEL TABLERO:" + tablero.obtenerTableroComoString());
        return true;
    }

    // Interrupcion del servidor (SIGINT): cierre ordenado, sin un ganador
    if (interrumpida.load()) {
        resultado = ResultadoPartida::INTERRUMPIDA;
        juegoActivo = false;

        std::cout << "[MESA " << idMesa << "] Partida interrumpida por el servidor (SIGINT). Sin ganador." << std::endl;

        escribirLog("\n=== FIN DE LA PARTIDA (INTERRUMPIDA) ===");
        escribirLog("Motivo: El servidor recibio SIGINT; cierre ordenado, sin ganador.");
        escribirLog("Movimientos totales registrados: " + std::to_string(totalMovimientos.load()));
        escribirLog("ESTADO FINAL DEL TABLERO:" + tablero.obtenerTableroComoString());
        return true;
    }

    return false;
}