#include <fstream>  // Trabajar con archivos
#include <iostream> // I/0 Default del sistema
#include <string>   // Trabajar con strings

using namespace std;

// Verificar que el archivo existe
bool verificar_archivo(const string &nombre) {
  ifstream archivo(nombre);
  return archivo.good(); // Retorna true si el archivo existe
}

bool verificar_parametros(int argc, char **args) {
  if (argc < 3) {
    cout << "Uso: ./planificador <archivo.txt> <Limite>\n";
    return false;
  }

  // Convertimos el argumento de texto a número entero
  int limite = stoi(args[2]);
  if (limite <= 0) {
    cout << "No puede tener un limite inferior o igual a cero\n";
    return false;
  }

  if (!verificar_archivo(args[1])) {
    cout << "Archivo no encontrado\n";
    return false;
  }

  return true;
}

int main(int argc, char **args) {
  if (!verificar_parametros(argc, args)) {
    return -1;
  }

  cout << "Parametros validos y encontrados\n";
  return 0;
}
