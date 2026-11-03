#ifndef CONFIGURACION_HPP
#define CONFIGURACION_HPP

#include <string>
#include <vector>

struct ConfigJuego {
    int delayMin = 50;       // ms 
    int delayMax = 300;      // ms 
    unsigned int seed = 45;  // Semilla aleatoria por defecto
    
    int posPuma = 0;         // Vertice del trono por defecto
    std::vector<int> posPerros = {
        16, 17, 18, 19, 20,
        21, 22, 23, 24, 25, 26, 27
    };                      // 12 perros por defecto

    // inyeccion de fallos para poder probar el abandono
    // simula la caida de un jugador a mitad de partida, porque por defecto no falla nadie.
    int falloJugador = 0;    // 0 = ninguno, 1 = cae el jugador Puma, 2 = cae el jugador Perros
    int falloMesa = 0;       // mesa afectada (si es 0 son todas las mesas)
    int falloTrasMs = 1000;  // tiempo en ms desde el inicio de la partida hasta que ocurre la caida
};

class ParserConfiguracion {
public:
    static bool cargarDesdeArchivo(const std::string& ruta, ConfigJuego& config);
};

#endif // CONFIGURACION_HPP