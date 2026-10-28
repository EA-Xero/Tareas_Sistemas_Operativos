#include <iostream>
#include <cstdlib>
#include "Configuracion.hpp"
#include "Matchmaker.hpp"

int main(int argc, char* argv[]) {
    //Verificacion basica de argumentos
    if (argc < 3) {
        std::cout << "Uso: " << argv[0] << " N M [config.txt]\n";
        return 1;
    }

    // Cantidad de mesas y jugadores
    int nJugadores = std::atoi(argv[1]);
    int mMesas = std::atoi(argv[2]);

    // Verificacion de parametros para evitar errores o bucles
    if (numJugadores < 2 || numJugadores % 2 != 0) {
        std::cerr << "Error: El número de jugadores (N) debe ser par y mayor o igual a 2." << std::endl;
        return 1;
    }

    if (maxMesas <= 0) {
        std::cerr << "Error: El número máximo de mesas (M) debe ser estrictamente mayor a 0." << std::endl;
        return 1;
    }

    // Declaracion y definicion del archivo de configuracion
    ConfigJuego config;
    if (argc >= 4) {
        ParserConfiguracion::cargarDesdeArchivo(argv[3], config);
    }

    //Inicia el torneo
    Matchmaker torneoServer(nJugadores, mMesas, config);
    torneoServer.iniciarTorneo();

    return 0;
}