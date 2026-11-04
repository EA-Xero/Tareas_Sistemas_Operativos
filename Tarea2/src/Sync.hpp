#ifndef SYNC_HPP
#define SYNC_HPP

#include <pthread.h>

// Envoltorio minimo sobre pthread_mutex_t (primitiva POSIX), porque el enunciado pedia sincronizacion con POSIX y no me quiero arriesgar a que nos bajen denuevo.
// Se inicializa en el constructor y se destruye en el destructor, por lo que ningun mutex
// queda huerfano aunque la mesa o el hilo que lo uso termine por excepcion (RAII).
class MutexPosix {
    pthread_mutex_t m;
public:
    MutexPosix() { pthread_mutex_init(&m, nullptr); }
    ~MutexPosix() { pthread_mutex_destroy(&m); }
    MutexPosix(const MutexPosix&) = delete;
    MutexPosix& operator=(const MutexPosix&) = delete;
    void lock() { pthread_mutex_lock(&m); }
    void unlock() { pthread_mutex_unlock(&m); }
};

// Equivalente a std::lock_guard. Toma el mutex al crearse y lo suelta al salir del scope
// (return, break o excepcion). Evita dejar un mutex tomado por un hilo que fallo.
class GuardPosix {
    MutexPosix& mtx;
public:
    explicit GuardPosix(MutexPosix& m) : mtx(m) { mtx.lock(); }
    ~GuardPosix() { mtx.unlock(); }
    GuardPosix(const GuardPosix&) = delete;
    GuardPosix& operator=(const GuardPosix&) = delete;
};

#endif // SYNC_HPP
