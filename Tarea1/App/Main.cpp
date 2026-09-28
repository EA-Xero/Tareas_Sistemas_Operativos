#include <fstream>  // Trabajar con archivos
#include <iostream> // I/O Default del sistema
#include <sstream>  // para hacer operaciones I/O en los strings
#include <string>   // Trabajar con strings
#include <vector>   // Arrays Dinamicos
#include <cstdlib>  // Para poder usar rand y srand en casos sin tiempo
#include <ctime>    // Para poder usar time en casos sin tiempo
#include <csignal>  // Para manejar SIGINT
#include <sys/wait.h> // Para waitpid
#include <unistd.h>   // Para write y _exit
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
struct Actividad {
    int id;
    string nombre;
    long tiempo_ms;
    vector<int> dependencias;
};

// Funcion auxiliar para limpiar espacios en blanco
string trim(const string &str) {
    size_t primero = str.find_first_not_of(" \t");
    if (primero == string::npos)
        return "";
    size_t ultimo = str.find_last_not_of(" \t");
    return str.substr(primero, (ultimo - primero + 1));
}

// Parsear dependencias sin importar si vienen con corchetes o dos puntos
vector<int> parsear_dependencias(string texto_dep) {
    vector<int> deps;
    texto_dep = trim(texto_dep);

    if (texto_dep.empty())
        return deps;

    if (texto_dep.front() == '[')
        texto_dep.erase(0, 1);
    if (!texto_dep.empty() && texto_dep.back() == ']')
        texto_dep.pop_back();

    stringstream ss(texto_dep);
    string item;

    while (getline(ss, item, ',')) {
        item = trim(item);
        if (!item.empty()) {
            deps.push_back(stoi(item));
        }
    }
    return deps;
}

// Lectura y formateo de archivo con manejo de datos faltantes y aleatorios
vector<Actividad> leer_actividades(const string &ruta_archivo) {
    vector<Actividad> lista_actividades;
    ifstream archivo(ruta_archivo);
    string linea;

    while (getline(archivo, linea)) {
        if (trim(linea).empty())
            continue;

        stringstream ss(linea);
        string id_str, nombre, tiempo_str, deps_str;

        if (getline(ss, id_str, ':') && getline(ss, nombre, ':')) {
            Actividad act;
            act.id = stoi(trim(id_str));
            act.nombre = trim(nombre);

            if (getline(ss, tiempo_str, ':')) {
                tiempo_str = trim(tiempo_str);

                if (tiempo_str.empty()) {
                    // Si viene vacio le tiramos un random entre 100 y 5000 ms
                    act.tiempo_ms = 100 + (rand() % 4901);
                } else {
                    act.tiempo_ms = stol(tiempo_str);
                }
            } else {
                // Si la linea corta antes, igual le ponemos el random por si acaso
                act.tiempo_ms = 100 + (rand() % 4901);
            }

            if (getline(ss, deps_str)) {
                act.dependencias = parsear_dependencias(deps_str);
            } else {
                act.dependencias = {};
            }

            lista_actividades.push_back(act);
        }
    }

    return lista_actividades;
}

// Vector global para llevar el catastro de hijos activos y matar a todos si salta la seremi (SIGINT)
vector<pid_t> pids_hijos_activos;

void manejar_sigint(int sig) {
    const char msg[] = "\n\nLlego la seremi ctmre corran\n";
    write(STDOUT_FILENO, msg, sizeof(msg) - 1);

    for (pid_t pid : pids_hijos_activos) {
        if (pid > 0) { kill(pid, SIGKILL); }
    }

    for (pid_t pid : pids_hijos_activos) {
        if (pid > 0) {
            int status;
            waitpid(pid, &status, 0);
        }
    }

    _exit(sig);
}

int main(int argc, char **args) {
    // Inicializamos la semilla aleatoria para los tiempos faltantes
    srand(time(nullptr));

    // Registramos la signal para la seremi
    signal(SIGINT, manejar_sigint);

    if (!verificar_parametros(argc, args)) {
        return -1;
    }

    cout << "Parametros correctos. Leyendo archivo...\n\n";

    vector<Actividad> actividades = leer_actividades(args[1]);

    // Imprimir para verificar que leyó correctamente y asigno los randoms si faltaban
    for (const auto &act : actividades) {
        cout << "ID: " << act.id << " | Nombre: " << act.nombre
             << " | Tiempo: " << act.tiempo_ms << "ms"
             << " | Dependencias: ";

        for (int dep : act.dependencias) {
            cout << dep << " ";
        }
        cout << "\n";
    }

    return 0;
}
