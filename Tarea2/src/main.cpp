#include <iostream>
#include "Tablero.hpp"

int main(int argc, char* argv[]) {
    std::cout << "[INFO] Iniciando prueba inicial del tablero Komikan...\n";
    
    // Verificación simple de argumentos por línea de comandos
    if (argc > 1) {
        std::cout << "[CONFIG] Se detecto argumento: " << argv[1] << " (Se procesara en la Fase de Configuración)\n";
    }

    Tablero tableroPrueba;
    tableroPrueba.imprimirTableroTextual();

    return 0;
}