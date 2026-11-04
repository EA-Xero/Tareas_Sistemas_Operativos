# Torneo de Komikan Concurrente - Tarea 2 Sistemas Operativos

Servidor de torneo multijugador en tiempo real basado en el juego tradicional mapuche "Komikan" (Puma vs. 12 Perros). Desarrollado en C++17, utilizando concurrencia por hilos y primitivas estrictas de POSIX (`pthread_mutex_t`, semaforos POSIX y manejo avanzado de señales).

## Requisitos de Compilacion y Ejecucion

El proyecto cuenta con un `Makefile` optimizado para entornos Unix que cumple con las banderas de compilacion estricta exigidas:

```bash
make                                  # Compila usando: g++ -Wall -Wextra -std=c++17 -pthread
./Juego/torneo N M [config.txt]             # Ejecucion (N = jugadores pares >= 2, M = mesas simultaneas > 0) [config.txt es el path al archivo de configuracion que se desea utilizar]
make clean                            # Limpieza completa (binarios, objetos y registros Logs/)
```

### Ejemplos de uso (se asume que se encuentra en la carpeta Tarea2):

```bash
./Juego/torneo 20 4 ./ConfigFiles/config1.txt
./juego/torneo 4 2 ./ConfigFiles/config_fallo_ejemplo.txt                            # Usa configuracion por defecto

```

Cada partida genera de forma automatica su propio archivo de registro detallado en la ruta `Logs/mesa_<id>.log`, conteniendo el estado del tablero tras cada movimiento y eventos del Juez.

## Archivo de Configuracion (config.txt)

El sistema permite parametrizar la simulacion mediante un archivo de texto con formato `clave = valor` (las lineas con `#` o `//` son ignoradas como comentarios):

| Clave | Descripcion | Valor por Defecto |
| --- | --- | --- |
| `delay_min`, `delay_max` | Rango de retardo en milisegundos entre decisiones de cada pieza | `50`, `300` |
| `seed` | Semilla para el generador de números pseudoaleatorios (`std::mt19937`) | `45` |
| `pos_puma` | Casilla inicial de ubicacion del Puma (0 a 27) | `0` (Vertice del trono) |
| `pos_perros` | Vector con las posiciones iniciales de los 12 perros | `16..27` (Fondo del tablero) |
| `fallo_jugador` | Inyeccion de fallos: `0` (ninguno), `1` (cae Puma), `2` (caen Perros) | `0` |
| `fallo_mesa` | Mesa afectada por la inyeccion de fallos (`0` = aplica a todas las mesas) | `0` |
| `fallo_tras_ms` | Tiempo en milisegundos de ejecucion antes de simular la desconexion | `1000` |

Nota de robustez: El parser incluye validaciones automaticas que corrigen anomalias en tiempo de ejecucion (por ejemplo, si `delay_min > delay_max`, se intercambian automaticamente).

## Arquitectura y Decisiones de Sincronizacion

### 1. Primitivas POSIX y RAII (Sync.hpp)

Para cumplir estrictamente con los requisitos de evaluacion, se desarrollo un envoltorio orientado a objetos sobre la API nativa de POSIX:

* `MutexPosix`: Envuelve un `pthread_mutex_t`, inicializandolos mediante `pthread_mutex_init` y destruyendolos de forma segura en el destructor (`pthread_mutex_destroy`).

* `GuardPosix`: Implementa el patron RAII equivalente a `std::lock_guard`, adquiriendo el cerrojo al instanciarse y liberandolo automaticamente al salir del ambito (`return`, `break` o excepciones), previniendo bloqueos huerfanos.

### 2. Control de Acceso al Tablero y Reglas

* **Exclusion Mutua Fina**: Cada instancia de `Tablero` protege su estructura de grafo y estados con un `MutexPosix` exclusivo.

* **Identidad Fija de Perros (`idPieza`)**: Cada uno de los 12 hilos de perros controla de forma univoca una pieza con identificador fijo (`0` a `11`). Si un perro es capturado por el Puma, el hilo detecta su eliminacion y finaliza ordenadamente.

* **Invariantes del Juez**: El hilo arbitro de cada mesa verifica ciclicamente la consistencia global del tablero (`pumas == 1` y `perros_en_tablero + capturados == 12`).

### 3. Servidor de Emparejamiento y Espera Pasiva (Matchmaker)

* **Llegada Concurrente**: Cada cliente se simula como un hilo independiente que se encola tras un retardo aleatorio (`semJugadores` y `mtxCola`).

* **Limite de Mesas (M)**: Se utiliza un semaforo POSIX (`sem_t semMesasDisponibles`) inicializado en M. Los hilos del servidor realizan una espera pasiva eficiente mediante `sem_wait` cuando todas las mesas estan ocupadas.

  ### 4. Resiliencia y Manejo de Señales (SIGINT)

* La señal de interrupcion de usuario (`Ctrl+C`) esta bloqueada globalmente en todos los hilos y es atendida de forma exclusiva por un hilo dedicado mediante **`sigwait`** y mascaras POSIX (`pthread_sigmask`). Al activarse, detiene la admision de nuevos clientes, notifica de forma ordenada a las mesas activas y libera todos los recursos del sistema operativo sin dejar fugas ni hilos colgados.


