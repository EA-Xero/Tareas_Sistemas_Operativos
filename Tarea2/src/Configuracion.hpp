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
};

class ParserConfiguracion {
public:
    static bool cargarDesdeArchivo(const std::string& ruta, ConfigJuego& config);
};

#endif // CONFIGURACION_HPP