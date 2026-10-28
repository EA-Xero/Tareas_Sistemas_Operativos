#include <iostream>
#include <cstdlib>
#include "Tablero.hpp"
#include "Configuracion.hpp"
#include "MesaJuego.hpp"

using namespace std;
int main(int argc, char* argv[]) {
    // Invocacion requerida: ./torneo N M [config.txt]
    if (argc < 3) {
        cout << "Uso: " << argv[0] << " N M [config.txt]\n";
        cout << "  N: Numero de clientes/jugadores\n";
        cout << "  M: Numero maximo de mesas concurrentes\n";
        cout << "  config.txt: Archivo opcional de configuracion\n";
        return 1;
    }

    int nJugadores = atoi(argv[1]);
    int mMesas = atoi(argv[2]);

    cout << "=== PARÁMETROS DEL TORNEO ===\n";
    cout << "Jugadores (N): " << nJugadores << "\n";
    cout << "Mesas Maximas (M): " << mMesas << "\n";

    ConfigJuego config;
    if (argc >= 4) {
        ParserConfiguracion::cargarDesdeArchivo(argv[3], config);
    } else {
        cout << "[CONFIG] Sin archivo de configuracion. Usando valores predeterminados.\n";
    }

    cout << "Configuracion de retardo: [" << config.delayMin << " ms - " << config.delayMax << " ms]\n";
    cout << "Seed: " << config.seed << "\n";

    MesaJuego mesa(config);
    mesa.iniciarPartidaPrueba(3);

    return 0;
}