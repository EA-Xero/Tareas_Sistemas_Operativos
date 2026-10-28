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