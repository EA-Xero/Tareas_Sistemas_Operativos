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

// Revisa si faltan parametros segun la rubrica, o si estan mal escritos/formateados
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

    //Leere linea por linea, guarda los resultados en el vector de actividades
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

// Vector global para el manejo de hijos y la senal de la seremi
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
                simulacion_activa = true; 
            }
        }

        // Lanzar nuevas tareas si hay espacio segun el limite K
        for (auto &act : actividades) {
            if (act.estado == 0 && corriendo_actuales < limite_k) {
                if (dependencias_cumplidas(act, actividades)) {
                    if (pipe(act.pipe_fd) < 0) {
                        perror("Error al crear pipe");
                        continue;
                    }

                    pid_t pid = fork();
                    if (pid < 0) {
                        perror("Error en fork");
                        break;
                    } else if (pid == 0) {
                        signal(SIGINT, SIG_DFL); // Evitar colision con la señal de la seremi en el hijo (el codigo anterior duplicaba el output)
                        close(act.pipe_fd[0]);
                        usleep(act.tiempo_ms * 1000);
                        char exito = '1';
                        write(act.pipe_fd[1], &exito, 1);
                        close(act.pipe_fd[1]);
                        _exit(0);
                    } else {
                        act.pid = pid;
                        act.estado = 1;
                        corriendo_actuales++;
                        pids_hijos_activos.push_back(pid);
                        close(act.pipe_fd[1]);
                        cout << "[INICIO] Actividad " << act.id << " (" << act.nombre 
                             << ") corriendo con PID " << pid << " [Tiempo: " << act.tiempo_ms << "ms]\n";
                    }
                }
            }
        }

        // Primero revisamos si hay alguno terminado de forma no bloqueante
        bool algun_hijo_revisado = false;
        for (auto &act : actividades) {
            if (act.estado == 1) {
                int status;
                pid_t resultado = waitpid(act.pid, &status, WNOHANG);

                if (resultado > 0) {
                    algun_hijo_revisado = true;
                    auto it_p = find(pids_hijos_activos.begin(), pids_hijos_activos.end(), act.pid);
                    if (it_p != pids_hijos_activos.end()) {
                        pids_hijos_activos.erase(it_p);
                    }

                    char buf = '0';
                    read(act.pipe_fd[0], &buf, 1);
                    close(act.pipe_fd[0]);

                    if (WIFEXITED(status) && WEXITSTATUS(status) == 0 && buf == '1') {
                        act.estado = 2; 
                        cout << "[TERMINADO] Actividad " << act.id << " (" << act.nombre << ") completada con exito.\n";
                    } else {
                        act.estado = 3; 
                        cout << "[ERROR] Actividad " << act.id << " (" << act.nombre << ") fallo durante su ejecucion.\n";
                        abortar_dependientes(act.id, actividades);
                    }
                }
            }
        }
	/*
	 *Hay un problema que se arreglo ahora del codigo anterior, consumia mucha cpu por que el padre constantemente tenia que retornar al bucle y revisar si los hijos habian terminado,
	 con una prueba de muchas actividades casi se me que la pc :/, bueno tampoco asi pero ya entiendes.

	 Ademas comparando con la rubrica, se menciona que no se puede hacer busy-waiting (el usleep() ), por lo que me puse a eliminarlos y cambiarlos
	 * */

        // SI YA ESTAMOS AL LIMITE DE CONCURRENCIA (K) y ningun hijo termino en este ciclo,
        // en vez de quemar CPU haciendo bucle, esperamos pasivamente a que CUALQUIER hijo muera.
        if (!algun_hijo_revisado && corriendo_actuales >= limite_k && simulacion_activa) {
            int status;
            pid_t pid_terminado = wait(&status); // Bloqueo eficiente del SO (Cero Busy-Waiting)
            if (pid_terminado > 0) {
                // Buscamos cual de los activos corresponde a este PID para procesarlo en la siguiente iteracion
                for (auto &act : actividades) {
                    if (act.estado == 1 && act.pid == pid_terminado) {
                        auto it_p = find(pids_hijos_activos.begin(), pids_hijos_activos.end(), pid_terminado);
                        if (it_p != pids_hijos_activos.end()) {
                            pids_hijos_activos.erase(it_p);
                        }

                        char buf = '0';
                        read(act.pipe_fd[0], &buf, 1);
                        close(act.pipe_fd[0]);

                        if (WIFEXITED(status) && WEXITSTATUS(status) == 0 && buf == '1') {
                            act.estado = 2;
                            cout << "[TERMINADO] Actividad " << act.id << " (" << act.nombre << ") completada con exito.\n";
                        } else {
                            act.estado = 3;
                            cout << "[ERROR] Actividad " << act.id << " (" << act.nombre << ") fallo durante su ejecucion.\n";
                            abortar_dependientes(act.id, actividades);
                        }
                    }
                }
            }
        }
    }
    // ademas, eliminamos los casos de prueba anteriores (por ejemplo antes habia un 5% de posibilidad de que una tarea fallara solo para probar la cancelacion de tareas en cascada
    cout << "TIKI TIKI TI!";
    return 0;
}
