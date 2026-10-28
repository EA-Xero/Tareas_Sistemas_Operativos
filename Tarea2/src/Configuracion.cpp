#include "Configuracion.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

//Archivo encargado de cargar la configuracion inicial de cada mesa, parsear y aplicar

// Auxiliar para quitar espacios en blanco al inicio y final
static std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

// parsea el archivo.txt de configuracion y le asigna los valores dentro a variables internas del juego
bool ParserConfiguracion::cargarDesdeArchivo(const std::string& ruta, ConfigJuego& config) {
    std::ifstream archivo(ruta);
    if (!archivo.is_open()) {
        std::cerr << "[ADVERTENCIA] No se pudo abrir el archivo de configuracion: " << ruta 
                  << ". Usando valores por defecto.\n";
        return false;
    }

    std::string linea;
    while (std::getline(archivo, linea)) {
        linea = trim(linea);

        // Ignorar lineas vacias o comentarios
        if (linea.empty() || linea[0] == '#' || (linea.size() >= 2 && linea[0] == '/' && linea[1] == '/')) {
            continue;
        }

        std::istringstream streamLinea(linea);
        std::string clave;
        if (std::getline(streamLinea, clave, '=')) {
            clave = trim(clave);
            std::string valorStr;
            if (std::getline(streamLinea, valorStr)) {
                valorStr = trim(valorStr);
                std::istringstream streamValor(valorStr);

                if (clave == "delay_min") {
                    streamValor >> config.delayMin;
                } else if (clave == "delay_max") {
                    streamValor >> config.delayMax;
                } else if (clave == "seed") {
                    streamValor >> config.seed;
                } else if (clave == "pos_puma") {
                    streamValor >> config.posPuma;
                } else if (clave == "pos_perros") {
                    config.posPerros.clear();
                    int pos;
                    while (streamValor >> pos) {
                        config.posPerros.push_back(pos);
                    }
                }
            }
        }
    }

    archivo.close();
    std::cout << "[CONFIG] Archivo '" << ruta << "' cargado exitosamente.\n";
    return true;
}