#include <iostream>
#include <time.h> // nanosleep

int main() {
    std::cout << "Esperando 1.5 segundos..." << std::endl;

    struct timespec ts;
    ts.tv_sec = 1;   // 1 segundo
    ts.tv_nsec = 500000000; // 0.5 segundos (500 millones de nanosegundos)

    nanosleep(&ts, NULL);

    std::cout << "Listo!" << std::endl;
    return 0;
}
