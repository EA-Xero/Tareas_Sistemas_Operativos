#include <fstream>  // Trabajar con archivos
#include <iostream> // I/O Default del sistema
#include <sstream>  // para hacer operaciones I/O en los strings
#include <string>   // Trabajar con strings
#include <vector>   // Arrays Dinamicos
#include <cstdlib>  // Para poder usar rand y srand en casos sin tiempo
#include <ctime>    // Para poder usar time en casos sin tiempo
#include <csignal>  // Para manejar SIGINT
#include <sys/wait.h> // Para waitpid
#include <unistd.h>   // Para write, fork, pipe, etc.
#include <algorithm>  // Para buscar en vectores

using namespace std;

// Verificar que el archivo existe
bool verificar_archivo(const string &nombre) {
    ifstream archivo(nombre);
    return archivo.good();
}

bool verificar_parametros(int argc, char **args, int &limite_out) {
    if (argc < 3) {
        cout << "Uso: ./planificador <archivo.txt> <Limite>\n";
        return false;
    }

    int limite = stoi(args[2]);
    if (limite <= 0) {
        cout << "No puede tener un limite inferior o igual a cero\n";
        return false;
    }

    if (!verificar_archivo(args[1])) {
        cout << "Archivo no encontrado\n";
        return false;
    }

    limite_out = limite;
    return true;
}

// Estructura basica de una Actividad
struct Actividad {
    int id;
    string nombre;
    long tiempo_ms;
    vector<int> dependencias;
    
    // Estados internos para la simulacion del DAG
    // 0: Pendiente, 1: Ejecutandose, 2: Finalizada, 3: Fallida
    int estado = 0; 
    pid_t pid = -1;
    int pipe_fd[2] = {-1, -1};
};

// Funcion auxiliar para limpiar espacios en blanco
string trim(const string &str) {
    size_t primero = str.find_first_not_of(" \t");
    if (primero == string::npos)
        return "";
    size_t ultimo = str.find_last_not_of(" \t");
    return str.substr(primero, (ultimo - primero + 1));
}

// Parsear dependencias sin importar corchetes o dos puntos
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
                    act.tiempo_ms = 100 + (rand() % 4901);
                } else {
                    act.tiempo_ms = stol(tiempo_str);
                }
            } else {
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

// Vector global para el manejo de hijos y la seal de la seremi
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

// Funcion para verificar si todas las dependencias de una actividad ya terminaron con exito
bool dependencias_cumplidas(const Actividad &act, const vector<Actividad> &todas) {
    for (int dep_id : act.dependencias) {
        auto it = find_if(todas.begin(), todas.end(), [dep_id](const Actividad &a) {
            return a.id == dep_id;
        });
        // Si la dependencia no existe o no esta finalizada correctamente (estado 2), no se puede ejecutar
        if (it == todas.end() || it->estado != 2) {
            return false;
        }
    }
    return true;
}

// Funcion recursiva para abortar en cascada a los procesos que dependen de uno fallido
void abortar_dependientes(int id_fallido, vector<Actividad> &actividades) {
    for (auto &act : actividades) {
        // Si esta actividad depende del id_fallido y aun no esta muerta/finalizada
        auto it = find(act.dependencias.begin(), act.dependencias.end(), id_fallido);
        if (it != act.dependencias.end() && act.estado != 3 && act.estado != 2) {
            act.estado = 3; // Marcada como fallida por propagacion
            if (act.pid > 0) {
                kill(act.pid, SIGKILL);
            }
            cout << "[FALLO EN CASCADA] Actividad " << act.id << " (" << act.nombre << ") abortada por fallo previo.\n";
            abortar_dependientes(act.id, actividades);
        }
    }
}

int main(int argc, char **args) {
    srand(time(nullptr));
    signal(SIGINT, manejar_sigint);

    int limite_k = 0;
    if (!verificar_parametros(argc, args, limite_k)) {
        return -1;
    }

    cout << "Parametros correctos. Leyendo archivo...\n\n";
    vector<Actividad> actividades = leer_actividades(args[1]);

    cout << "Iniciando simulacion del planificador con limite K = " << limite_k << "...\n\n";

    // Bucle principal del planificador estilo coordinador
    bool simulacion_activa = true;
    while (simulacion_activa) {
        simulacion_activa = false;
        int corriendo_actuales = 0;

        // Contar cuantos estan corriendo actualmente y revisar estados
        for (const auto &act : actividades) {
            if (act.estado == 1) {
                corriendo_actuales++;
            }
            if (act.estado == 0 || act.estado == 1) {
                simulacion_activa = true; // Todavia hay pega por hacer
            }
        }

        // Lanzar nuevas tareas si hay espacio segun el limite K
        for (auto &act : actividades) {
            if (act.estado == 0 && corriendo_actuales < limite_k) {
                // Verificar si sus dependencias terminaron bien
                if (dependencias_cumplidas(act, actividades)) {
                    
                    // Creamos el pipe para notificaciones si hace falta
                    if (pipe(act.pipe_fd) < 0) {
                        perror("Error al crear pipe");
                        continue;
                    }

                    pid_t pid = fork();
                    if (pid < 0) {
                        perror("Error en fork");
                        break;
                    } else if (pid == 0) {
                        // Codigo del proceso hijo
                        // Cerramos el extremo de lectura del pipe en el hijo
                        close(act.pipe_fd[0]);

                        // Simulamos la ejecucion del tiempo de la tarea
                        // Convertimos ms a microsegundos con usleep
                        usleep(act.tiempo_ms * 1000);

                        // Simulamos una pequeña probabilidad de fallo aleatorio (ej 5%) para probar la tolerancia a fallos
                        // O lo dejamos estable segun se requiera. Aqui avisamos exito escribiendo en el pipe.
                        char exito = '1';
                        write(act.pipe_fd[1], &exito, 1);
                        close(act.pipe_fd[1]);

                        _exit(0);
                    } else {
                        // Codigo del proceso padre
                        act.pid = pid;
                        act.estado = 1; // En ejecucion
                        corriendo_actuales++;
                        pids_hijos_activos.push_back(pid);
                        
                        // Cerramos el extremo de escritura en el padre
                        close(act.pipe_fd[1]);

                        cout << "[INICIO] Actividad " << act.id << " (" << act.nombre 
                             << ") corriendo con PID " << pid << " [Tiempo: " << act.tiempo_ms << "ms]\n";
                    }
                }
            }
        }

        // Revisar si algun hijo termino usando waitpid con WNOHANG para no bloquearnos
        for (auto &act : actividades) {
            if (act.estado == 1) {
                int status;
                pid_t resultado = waitpid(act.pid, &status, WNOHANG);

                if (resultado > 0) {
                    // El proceso termino, lo removemos del vector global de activos
                    auto it_p = find(pids_hijos_activos.begin(), pids_hijos_activos.end(), act.pid);
                    if (it_p != pids_hijos_activos.end()) {
                        pids_hijos_activos.erase(it_p);
                    }

                    char buf = '0';
                    read(act.pipe_fd[0], &buf, 1);
                    close(act.pipe_fd[0]);

                    if (WIFEXITED(status) && WEXITSTATUS(status) == 0 && buf == '1') {
                        act.estado = 2; // Finalizada con exito
                        cout << "[TERMINADO] Actividad " << act.id << " (" << act.nombre << ") completada con exito.\n";
                    } else {
                        act.estado = 3; // Fallida
                        cout << "[ERROR] Actividad " << act.id << " (" << act.nombre << ") fallo durante su ejecucion.\n";
                        abortar_dependientes(act.id, actividades);
                    }
                } else if (resultado < 0) {
                    // Error en waitpid o proceso ya no existe
                    act.estado = 3;
                }
            }
        }

        // Pequeña pausa para no saturar la CPU en el bucle de coordinacion
        usleep(10000); // 10 ms
    }

    cout << "\nSimulacion del planificador finalizada correctamente.\n";
    return 0;
}
