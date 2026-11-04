#include "Matchmaker.hpp"
#include <iostream>
#include <thread>
#include <algorithm>
#include <cerrno>
#include <pthread.h>
#include <system_error>
#include <functional> // necesario para function<void()>
#include <random>
#include <chrono>

// Archivo encargado de actuar como Juez y Emparejador de jugadores, maneja las mesas

/* Esta wea funciona, y es un C++ igual medio avanzado. Tuve que estudiar para esto.
G4G, W3Schools, stackoverflow y cppreference me carrearon de pana aca. Te explico para que es esta wea, incluso si la explicacion viene abajo en terminos correctos: 

Cree un namespace (para ordenar y agrupar la logica) sin nombre por dos razones, la primera
para que esta logica exista SOLO DENTRO DE ESTE ARCHIVO (internal linkage),
y para ocultarla de los otros archivos, porque solo este .cpp se encarga del manejo.
El namespace mas popular es el de std pero es publico, mientras que este es privado.

La clase AlSalir (RAII, una tecnica de OOP, buscala) actua como una red de seguridad basicamente, porque su proposito 
es prevenir que el Matchmaker intente notificar a una mesa que ya no exista (lo que causaria un 
error de segmentacion). Todos los parametros estan vinculados a la existencia de una mesa, 
y se borran cuando la mesa se va.

std::move(fn) es basicamente un static_cast. transforma el objeto en un rvalue (valor apunto de vencer), cambiando la interpretacion del objeto en si. Para explicar como funciona, necesitas entender fundamentalmente como el compilador trabaja las variables segun su definicion.
Basicamente, si hiciera f = fn, el compilador asume que fn seguira existiendo despues, por lo que futuros llamados lo tratarian como copias para no arriesgar a vaciar el valor original, causando un uso absurdo de recursos.
pero haciendolo un rvalue, el compilador sabe que no va a existir despues, por lo que simplemente mueve los punteros y recursos de la funcion hacia el objeto, haciendo que el objeto original (fn) quede en un estado sin especificar (como un NULLPTR), y f con los punteros y memoria que este tenia. Entonces, como literalmente dice el nombre, mueves los recursos de la funcion hacia el objeto, en vez de crear la variable y copiarle los valores de la funcion. Esto hace que la limpieza de recursos sea mucho mas rapida y ligera, pues no desperdicias memoria en crear copias de algo que se va a cerrar inmediatamente despues.

Si lo quieres leer por tu cuenta:
https://www.geeksforgeeks.org/cpp/resource-acquisition-is-initialization/
https://en.cppreference.com/cpp/utility/move
*/

namespace {
// Ejecuta una funcion al salir del scope (incluso por excepcion).
class AlSalir {
    std::function<void()> f; 
public:
    explicit AlSalir(std::function<void()> fn) : f(std::move(fn)) {} 
// explicit evita conversiones implicitas. AlSalir(F fn) recibe 
    // la funcion de limpieza y f(move(fn)) la mueve al atributo f, haciendo que sea mas segura a la hora de tomar funciones lambda por referencia (&mesa).
    // los dos puntos sirven para inicializar la variable antes de crearla, por lo que f se crea una vez ya asignada a la funcion de limpieza.
    ~AlSalir() { if (f) f(); } // Destructor que ejecuta la funcion de limpieza. Se ejecuta cuando sale de su scope (sea termino, return, o exception).
    AlSalir(const AlSalir&) = delete; // constructor de copia de la clase
    AlSalir& operator=(const AlSalir&) = delete; // operador de asignacion de la clase
// Se borran ambos constructor de copia y operador de asignacion. Dado que cada objeto AlSalir se vincula a la existencia de una mesa, borrarlos
    // asegura que los objetos sean unicos e intransferibles por mesa.
};
}

// Constructor
Matchmaker::Matchmaker(int N, int M, const ConfigJuego& cfg)
    : totalJugadores(N), maxMesasConcurrentes(M), config(cfg) {
    sem_init(&semMesasDisponibles, 0, maxMesasConcurrentes);
    sem_init(&semJugadores, 0, 0);
}

// Destructor
Matchmaker::~Matchmaker() {
    detenerManejoSenales(); // idempotente (busca la definicion si no sabes). por si iniciarTorneo salio antes de tiempo
    sem_destroy(&semMesasDisponibles);
    sem_destroy(&semJugadores);
}

/* SIGINT
En vez de un handler clasico (que solo puede usar funciones async-signal-safe y no
puede tomar mutex ni tocar el registro de mesas), hice que SIGINT se bloqueara en TODOS los hilos y que solo un
hilo dedicado lo esperara con sigwait(), para que el codigo de cierre corriera en contexto normal.

Eso si, la funcion de manejo de senales debe llamarse ANTES de crear cualquier otro hilo, porque la mascara bloqueada se hereda 
a los hilos hijos (mesas, jugadores, juez). Asi SIGINT nunca se entrega a un hilo de juego y solo lo consume el hilo con sigwait.

En resumen: Un puro hilo espera el SIGINT, y cuando detecta un Ctrl+C, se despierta la funcion
bucleSenales, quien llama a notificarInterrupcion(), y se pone en marcha toda la logica de cierre. */

void Matchmaker::iniciarManejoSenales() {
    sigemptyset(&mascaraSenales);
    sigaddset(&mascaraSenales, SIGINT);
    pthread_sigmask(SIG_BLOCK, &mascaraSenales, nullptr);
    hiloSenales = std::thread(&Matchmaker::bucleSenales, this);
}

void Matchmaker::bucleSenales() {
    int sig = 0;
    while (true) {
        if (sigwait(&mascaraSenales, &sig) != 0) return; // error inesperado: no girar en vacio
        if (finalizandoSenales.load()) return;           // despertado por detenerManejoSenales()

        if (!detener.load()) { notificarInterrupcion(); } 
        else { std::cout << "\n[SIGINT] Cierre ya en curso, espere a que terminen las partidas...\n"; }
    }
}

// 1) deja de aceptar jugadores, 2) despierta al Matchmaker si dormia, 3) avisa a cada partida activa
void Matchmaker::notificarInterrupcion() {
    detener.store(true);
    std::cout << "\n[SIGINT] Interrupcion recibida. No se aceptaran mas jugadores; "
                 "cerrando partidas activas de forma ordenada...\n";

    // Si el Matchmaker esta bloqueado en sem_wait esperando slot, esto lo despierta
    sem_post(&semMesasDisponibles);
    // Y si estaba esperando a que llegara un jugador, tambien
    sem_post(&semJugadores);

    GuardPosix lock(mtxMesasActivas);
    for (MesaJuego* mesa : mesasActivas) {
        mesa->solicitarDetencion();
    }
}

// Termina el hilo de senales sin esperar un Ctrl+C real, enviandole SIGINT a si mismo
// esto es necesario porque el hilo en si esta dormido y fuera del scope del procesador, por lo que
// hay que darle un sigint falso para que se cierre. si no, el programa nunca podria hacerle join a 
// este hilo porque no es capaz de despertar solo
void Matchmaker::detenerManejoSenales() {
    if (!hiloSenales.joinable()) return;
    finalizandoSenales.store(true);
    pthread_kill(hiloSenales.native_handle(), SIGINT);
    hiloSenales.join();
    // La mascara de SIGINT del hilo principal queda bloqueada a proposito, porque restaurarla con una
    // senal pendiente mataria el proceso con la accion por default justo al terminar
}

// Una mesa se registra al crearse. Si SIGINT llego antes del registro, se le avisa de inmediato
// (el chequeo de "detener" y el registro ocurren bajo el mismo mutex que usa notificarInterrupcion,
// para que ninguna mesa pueda quedar sin avisar).
void Matchmaker::registrarMesa(MesaJuego* mesa) {
    GuardPosix lock(mtxMesasActivas);
    mesasActivas.push_back(mesa);
    if (detener.load()) mesa->solicitarDetencion(); // si lee el valor de variable y es true, se solicita detencion
}

void Matchmaker::desregistrarMesa(MesaJuego* mesa) {
    GuardPosix lock(mtxMesasActivas);
    mesasActivas.erase(std::remove(mesasActivas.begin(), mesasActivas.end(), mesa), mesasActivas.end());
}

// sem_wait puede devolver -1/EINTR si una senal interrumpe la espera; ignorar ese retorno
// (como hacia el codigo original) otorgaria un slot que no existe y superaria el limite M.
bool Matchmaker::esperarSlot() {
    while (sem_wait(&semMesasDisponibles) == -1) {
        if (errno != EINTR){ return false; }
    }
    if (detener.load()) {
        sem_post(&semMesasDisponibles); // se desperto por SIGINT, por lo que no se usa el slot
        return false;
    }
    return true;
}

// Rutina de cada cliente: llega tras un retardo aleatorio (0-200 ms) y se encola.
// Tras SIGINT ya no llegan mas jugadores.
void Matchmaker::llegadaJugador(int id) {
    std::mt19937 rng(config.seed + id);
    std::uniform_int_distribution<int> dist(0, 200);
    std::this_thread::sleep_for(std::chrono::milliseconds(dist(rng)));
    if (detener.load()) return;
    {
        GuardPosix lock(mtxCola);
        colaJugadores.push(id);
    }
    std::cout << "[MATCHMAKER] Llega el jugador " << id << ".\n";
    sem_post(&semJugadores); // avisa al Matchmaker que hay un jugador mas en cola
}

// Espera pasiva (sem_wait) hasta que haya un jugador en cola y lo saca. false = SIGINT o error.
bool Matchmaker::esperarJugador(int& id) {
    while (sem_wait(&semJugadores) == -1) {
        if (errno != EINTR) return false;
    }
    if (detener.load()) return false;
    GuardPosix lock(mtxCola);
    id = colaJugadores.front();
    colaJugadores.pop();
    return true;
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

    // bloquear SIGINT y levantar el hilo que lo espera antes de crear mas hilos
    iniciarManejoSenales();

    // Llegada concurrente: un hilo cliente por jugador (despues de iniciarManejoSenales, hereda la mascara)
    int clientesLanzados = 0;
    for (int j = 1; j <= totalJugadores; ++j) {
        try {
            hilosClientes.emplace_back(&Matchmaker::llegadaJugador, this, j);
            ++clientesLanzados;
        } catch (const std::system_error& e) {
            std::cerr << "[MATCHMAKER] No se pudo crear el cliente " << j << ": " << e.what() << "\n";
            break;
        }
    }

    // Calcula total de emparejamiento posible
    int parejasTotales = clientesLanzados / 2;
    std::vector<std::thread> hilosMesas;
    int mesasLanzadas = 0;


    for (int i = 0; i < parejasTotales; ++i) {
        // tras SIGINT no llegan mas jugadores
        if (detener.load()) break;

        // Empareja a los dos siguientes jugadores que lleguen: el 1ro juega de Puma y el 2do de Perros
        int idPuma = 0, idPerros = 0;
        if (!esperarJugador(idPuma) || !esperarJugador(idPerros)) break;
        int idMesa = i + 1;
        std::cout << "\n[MATCHMAKER] Pareja formada: Jugador " << idPuma << " (Puma) vs Jugador "
                  << idPerros << " (Perros).\n";

        //Espera con semaforo si se llega al limite de mesas
        std::cout << "\n[MATCHMAKER] Esperando slot para Mesa " << idMesa << "...\n";
        if (!esperarSlot()) break; // Espera pasiva; false = SIGINT (o error) mientras esperaba

        //Si hay mesa disponible, inicializa la mesa
        try {
        hilosMesas.push_back(std::thread([this, idMesa, idPuma, idPerros]() {
            std::cout << "[MATCHMAKER] Slot otorgado. Iniciando Mesa " << idMesa << ".\n";
            try {
                MesaJuego mesa(idMesa, idPuma, idPerros, this->config);
                // Se registra para que SIGINT pueda notificarla; el guard la quita del registro
                // ANTES de que mesa se destruya (se destruye en orden inverso a la declaracion)
                registrarMesa(&mesa);
                AlSalir quitarRegistro([this, &mesa]() { desregistrarMesa(&mesa); });

                try {
                    mesa.iniciarPartida();
                } catch (const std::exception& e) {
                    // Fallo al crear hilos -> los ya creados siguen vivos, se cierran
                    std::cerr << "[MATCHMAKER] Mesa " << idMesa << " no pudo iniciar: " << e.what() << "\n";
                    mesa.abortarPorDesconexion();
                }
                mesa.esperarTermino();
                // desregistrarMesa(&mesa); // <- metodo antiguo no cerraba bien en casos de fallo, por lo que se cambio
            } catch (const std::exception& e) {
                std::cerr << "[MATCHMAKER] Mesa " << idMesa << " fallo: " << e.what() << "\n";
            }

        // La mesa avanza hasta que termine, luego se otorga el slot a otra mesa
            std::cout << "[MATCHMAKER] Mesa " << idMesa << " finalizada. Liberando slot...\n";
            sem_post(&(this->semMesasDisponibles)); // Liberar slot (siempre, incluso tras un fallo)
        }));
        ++mesasLanzadas;
        } catch (const std::system_error& e) {
            // No se pudo crear el hilo de la mesa -> devolver el slot y dejar de aceptar jugadores
            std::cerr << "[MATCHMAKER] No se pudo crear hilo para Mesa " << idMesa << ": " << e.what() << "\n";
            sem_post(&semMesasDisponibles);
            break;
        }
    }

    //Hace join de los hilos de mesas
    for (auto& h : hilosMesas) { if (h.joinable()) h.join(); }

    // Los clientes ya llegaron (o salieron por SIGINT): se les hace join
    for (auto& h : hilosClientes) { if (h.joinable()) h.join(); }

    // Todas las mesas terminaron, por lo que ya no se necesita el hilo de senales
    detenerManejoSenales();

    if (detener.load()) {
        std::cout << "\n[TORNEO] Interrumpido por SIGINT. Mesas iniciadas: " << mesasLanzadas
                  << " de " << parejasTotales << ". Recursos liberados.\n";
        return;
    }

    // Si no hay pareja disponible para un jugador
    if (clientesLanzados % 2 != 0) {
        std::cout << "\n[MATCHMAKER] Jugador " << clientesLanzados << " quedo sin pareja disponible.\n";
    }

    std::cout << "\n[TORNEO] Torneo completado exitosamente.\n";
}
