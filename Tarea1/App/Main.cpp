#include <fstream>  // Trabajar con archivos
#include <iostream> // I/0 Default del sistema
#include <sstream>  //para hacer operaciones I/O en los strings
#include <string>   // Trabajar con strings
#include <vector>   // Arrays Dinamicos
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

// Estructura basica de una Actividad
// ID_Actividad : Nombre_Actividad : tiempo_ms : [Dependencia1, Dependencia2,
// ...]
struct Actividad {
  int id;
  string nombre;
  long tiempo_ms;
  vector<int> dependencias;
};

// Funcion auxiliar para limpiar espacios en blanco, [ por que es tan dificil
// trabanar con strings en C :( ]
string trim(const string &str) {
  // buscar el primer espacio que no sea una tabulacion ni espacio en blanco
  size_t primero = str.find_first_not_of(" \t");
  // Si no la encuentra, retorna espacio en blanco
  if (primero == string::npos)
    return "";
  // Si la encuentra o no, busca la ultima posicion que no sea una tabulacion ni
  // espacio en blanco
  size_t ultimo = str.find_last_not_of(" \t");
  // retorna un substring, desde y hasta las posiciones distintas de espacio o
  // tabulos
  return str.substr(primero, (ultimo - primero + 1));
}

/*
 * En la guia las dependencias vienen de dos maneras:
 * [dep1,dep2] en la explicacion del formato
 * : dep1,dep2 en la declaracion de cada proceso, por lo que decidi hacer esta
 * funcion que va a formatear cualquiera de los dos casos
 */
vector<int> parsear_dependencias(string texto_dep) {
  vector<int> deps; // vector a retornar de dependencias
  texto_dep =
      trim(texto_dep); // quitamos espacios en blacno con la funcion anterior

  if (texto_dep.empty())
    return deps; // si viene vacio, retorna espacio en blanco

  // Quitar corchetes [] si existen
  if (texto_dep.front() == '[')
    texto_dep.erase(0, 1);
  if (!texto_dep.empty() && texto_dep.back() == ']')
    texto_dep.pop_back();

  // Objeto tipo string operable
  stringstream ss(texto_dep);
  string item;

  // Leer el string operable, separar por coma, cada item es una dependencia.
  // se pushea a deps, y retorna
  while (getline(ss, item, ',')) {
    item = trim(item);
    if (!item.empty()) {
      deps.push_back(stoi(item));
    }
  }
  return deps;
}

// Lectura y formateo de archivo:
// devuelve un vector con actividades validadas
vector<Actividad> leer_actividades(const string &ruta_archivo) {
  // Vector de actividades
  vector<Actividad> lista_actividades;
  // lectura de archivo
  ifstream archivo(ruta_archivo);
  string linea;

  // leer linea por linea el archivo, parar si ya no quedan.
  while (getline(archivo, linea)) {

    // Si la linea esta vacia, saltarsela
    if (trim(linea).empty())
      continue;

    // De esa linea, hacer un string manejable
    stringstream ss(linea);
    string id_str, nombre, tiempo_str, deps_str;

    // Leemos obligatoriamente los primeros 3 campos
    if (getline(ss, id_str, ':') && getline(ss, nombre, ':') &&
        getline(ss, tiempo_str, ':')) {

      Actividad act;
      act.id = stoi(trim(id_str));
      act.nombre = trim(nombre);
      act.tiempo_ms = stol(trim(tiempo_str));

      // Intentamos leer el cuarto campo; si no existe, deps_str queda vacío
      // (esto se añade por que algunas actividades vienen sin dependenciias)
      if (getline(ss, deps_str)) {
        act.dependencias = parsear_dependencias(deps_str);
      } else {
        act.dependencias = {}; // Sin dependencias
      }

      lista_actividades.push_back(act);
    }
  }

  return lista_actividades;
}

// Main po, que mas se puede explicar.. recibe un contador de argumentos, y un
// arreglo con ellos
int main(int argc, char **args) {

  // verificamos que los parametros esten correctamente formateados y no falten
  if (!verificar_parametros(argc, args)) {
    return -1;
  }

  cout << "Parametros correctos. Leyendo archivo...\n\n";

  vector<Actividad> actividades = leer_actividades(args[1]);

  // Imprimir para verificar que leyó correctamente
  for (const auto &act : actividades) {
    cout << "ID: " << act.id << " | Nombre: " << act.nombre
         << " | Tiempo: " << act.tiempo_ms << "ms"
         << " | Dependencias: ";

    for (int dep : act.dependencias) {
      cout << dep << " ";
    }
    cout << "\n";
  }
  // hasta ahora solo lectura de tareas
  return 0;
}
