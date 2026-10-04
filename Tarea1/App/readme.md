# Tarea 1: Planificador Dieciochero

Simulador y planificador de actividades para la Tarea 1 de Sistemas Operativos (Procesos, Tuberías y Señales), consistente de un organizador dieciochero. El programa lee un plan de actividades con dependencias entre ellas (un Grafo Acíclico Dirigido, o DAG) y las ejecuta como procesos hijos reales, respetando el orden de las dependencias y un límite de concurrencia K, además de no utilizando hilos (threads) y/o mecanismos de sincronización de hilos.

## Listado De archivos:
 * **main.cpp**: Contiene el código de toda la lógica utilizada en la tarea.
 * **plan.txt**: Contiene todas las tareas que el planificador debe ejecutar.
 * **readme.md**: Contiene la explicación de la tarea, del código, y de la lógica utilizada.


## Compilación y uso

El comando para compilar el programa es:
```bash
g++ -Wall -Wextra -std=c++17 -lpthread Main.cpp -o planificador 
```
Y el codigo requerido para ejecutarlo (asumiendo que el archivo plan.txt se encuentra dentro de la misma carpeta) es:
```bash
./planificador plan.txt K
```

- `plan.txt`: archivo que contiene las actividades a ejecutar.
- `K`: variable que representa la cantidad máxima de actividades que pueden estar ejecutándose al mismo tiempo (entero mayor que cero).

Si faltan argumentos, si `K` es menor o igual a cero ( <= 0 ), o si el archivo no existe, el programa muestra un mensaje y termina.

## Formato del archivo de entrada

Cada línea describe una actividad:

```
ID : Nombre : tiempo_ms : [Dep1, Dep2, ...]
```

Ejemplo:

```
1 : prender_carbon : 500 :
2 : comprar_carne : 1200 :
3 : comprar_pan : 300 :
4 : asar_longaniza : 800 : 1, 2
5 : armar_choripan : 250 : 3, 4
6 : servir_mesa : 100 : 5
```

Reglas que acepta el parser:

- El ID es alfanumérico, por eso se guarda como `string` y no como `int`.
- Los espacios alrededor de cada campo se ignoran.
- Si el tiempo está vacío o la línea termina antes de ese campo, se asigna un tiempo aleatorio entre 100 y 5000 ms.
- Las dependencias pueden venir con o sin corchetes y se separan por comas. Una lista vacía significa que la actividad no depende de nadie.
- Las líneas en blanco se ignoran.

## Idea general del diseño

El proceso principal actúa como **coordinador**. Nunca ejecuta el trabajo de una actividad: solo decide qué actividades pueden comenzar, crea un proceso hijo (`fork`) por cada una, les entrega sus insumos, espera a que terminen y registra sus resultados. Cada hijo simula su trabajo durmiendo el tiempo indicado (`usleep`) y devuelve un mensaje con su resultado.

Cada vuelta del bucle principal hace tres cosas, en este orden:

1. **Contar** cuántas actividades están corriendo y si queda trabajo pendiente.
2. **Lanzar** todas las actividades pendientes cuyas dependencias ya terminaron, mientras haya cupo bajo el límite K.
3. **Esperar** (bloqueado en `waitpid`) a que termine alguna actividad, y procesar su resultado.

## Estructura de cada Actividad

Cada línea del archivo `plan.txt` se convierte en una `Actividad`, donde cada actividad se compone de:

| Campo | Para qué sirve |
|---|---|
| `id`, `nombre` | Identificación de la actividad |
| `tiempo_ms` | Duración simulada |
| `dependencias` | IDs de las actividades que deben terminar antes |
| `estado` | 0 pendiente, 1 ejecutándose, 2 finalizada con éxito, 3 fallida |
| `pid` | PID del proceso hijo mientras corre |
| `pipe_padre_a_hijo[2]` | Tubería por la que el padre envía los insumos al hijo |
| `pipe_hijo_a_padre[2]` | Tubería por la que el hijo devuelve su resultado al padre |
| `insumo_generado` | Mensaje producido por la actividad al terminar |

El vector `actividades` modela el DAG completo: los nodos son las actividades y las aristas son los IDs listados en `dependencias`.

## Funciones principales

- `verificar_parametros` y `verificar_archivo`: validan los argumentos de la línea de comandos.
- `trim`, `parsear_dependencias` y `leer_actividades`: leen y limpian el archivo, y construyen el vector de actividades.
- `dependencias_cumplidas`: devuelve `true` solo si todas las dependencias de una actividad existen y están en estado 2. Una dependencia que no existe nunca se cumple.
- `obtener_insumos_dependencias`: arma el texto que se enviará al hijo, juntando el insumo de cada dependencia con el formato `[ID: insumo] | [ID: insumo]`. Si no hay dependencias, envía `Sin dependencias`.
- `enviar_mensaje` y `recibir_mensaje`: escriben y leen una línea completa por una tubería. `enviar_mensaje` repite el `write` hasta enviar todo. `recibir_mensaje` lee byte a byte hasta encontrar `\n`, quien actua como delimitador.
- `abortar_dependientes`: aborta en cascada (de forma recursiva) a todas las actividades que dependen directa o indirectamente de una actividad fallida.
- `manejar_sigint`: manejador de la señal Ctrl+C (simulando la llegada de la Seremi).

## Comunicación por tuberías (pipes)

Cada actividad tiene dos tuberías, una en cada dirección, creadas justo antes del `fork`:

```
        pipe_padre_a_hijo
PADRE ------------------------> HIJO     (insumos de las dependencias)

PADRE <------------------------ HIJO     (resultado de la actividad)
        pipe_hijo_a_padre               
```

Después del `fork`, cada lado cierra los extremos que no usa. Esto es importante, ya que si un extremo de escritura queda abierto por error, el lector nunca recibe fin de archivo y puede quedarse bloqueado para siempre.

El protocolo de funcionamiento es el siguiente:

1. El padre escribe en una línea los insumos de las dependencias y cierra su extremo de escritura.
2. El hijo lee esa línea, simula su trabajo con `usleep(tiempo_ms * 1000)` y escribe `OK:<nombre>` por su tubería.
3. El hijo termina con `_exit(0)`.
4. Cuando el padre detecta con `waitpid` que el hijo terminó, lee el mensaje, cierra la tubería y guarda el texto en `insumo_generado`.
5. Cuando una actividad dependiente se lance, ese insumo viajará en el mensaje de entrada de su hijo.

De esta forma el resultado de una actividad llega a sus dependientes a través de tuberías, con el padre como intermediario. Los mensajes son siempre una sola línea de texto.

## Control de concurrencia (límite K)

Antes de lanzar una actividad, el padre cuenta cuántas se encuentran en estado 1. Solo se lanza una nueva si `corriendo_actuales < K`, y el contador se incrementa en cada lanzamiento. Como el padre reduce ese conteo únicamente después de recoger al hijo con `waitpid`, nunca hay más de K hijos vivos al mismo tiempo.

### Sin espera activa (busy-waiting)

El padre no hace sondeos en un ciclo. Cuando no puede lanzar nada más, se bloquea en `waitpid(-1, &status, 0)` y el sistema operativo lo despierta solo cuando un hijo termina. Mientras espera, este *no* consume CPU.

### Sin condiciones de carrera

Toda la lógica de planificación corre en un único proceso y un único hilo (no se usan hilos [threads], por restricción del enunciado). Los hijos no comparten memoria de datos con el padre ni entre sí: solo se comunican por tuberías. Cada hijo trabaja con su propia copia de la memoria creada en el `fork`, y el padre es el único que modifica el estado de las actividades.

## Aislamiento de errores

Una actividad se considera fallida si su proceso no termina con código de salida 0, o si el padre no logra leer su mensaje de resultado. En caso que esto ocurra, se desencadenan los siguientes sucesos:

1. La actividad pasa a estado 3 y se imprime `[ERROR]`.
2. Se llama a `abortar_dependientes`, que marca como fallidas (estado 3) a todas las actividades que dependen de ella, de forma transitiva, imprimiendo `[FALLO EN CASCADA]`.
3. Las ramas del plan que no dependen de la actividad fallida siguen ejecutándose con normalidad.

El simulador no se cierra en caso de error, solo se corta la rama afectada.

También se manejan los errores de las llamadas al sistema (`pipe`, `fork`, `write`), cerrando las tuberías abiertas para no filtrar descriptores de archivo.

## Inspección de la Seremi (Ctrl+C)

Al inicio se registra `manejar_sigint` con `signal(SIGINT, ...)`. Cuando el usuario presiona Ctrl+C, el manejador:

1. Escribe un mensaje con `write` (que es seguro dentro de un manejador de señales, a diferencia de `cout`).
2. Envía `SIGKILL` a todos los PIDs guardados en la lista global `pids_hijos_activos`.
3. Espera a cada uno con `waitpid` para no dejar procesos zombie ni huérfanos.
4. Termina el programa con `_exit`.

Los hijos restauran el comportamiento por defecto de SIGINT (`SIG_DFL`) apenas nacen, para que el manejador no se ejecute también en ellos y duplique la salida.

## Casos especiales

- **Ciclos o dependencias inexistentes:** si no hay nada corriendo y aún quedan actividades pendientes, ninguna puede avanzar. El programa lo detecta, imprime `No hay actividades que puedan continuar` y termina el bucle en lugar de quedarse colgado.
- **Tiempos faltantes:** se generan con `rand()`, inicializado con `srand(time(nullptr))`.
- **Salida con `_exit`:** los hijos terminan con `_exit`, y no con `exit`, para no vaciar dos veces los buffers de salida heredados del padre.

## Ejemplo de ejecución

Con el plan de ejemplo en el enunciado y `K = 2`:

```
[INICIO] Actividad 1 (prender_carbon) corriendo con PID 322 [Tiempo: 500ms]
[INICIO] Actividad 2 (comprar_carne) corriendo con PID 323 [Tiempo: 1200ms]
[TERMINADO] Actividad 1 (prender_carbon) completada. Insumo: OK:prender_carbon
[INICIO] Actividad 3 (comprar_pan) corriendo con PID 324 [Tiempo: 300ms]
[TERMINADO] Actividad 3 (comprar_pan) completada. Insumo: OK:comprar_pan
[TERMINADO] Actividad 2 (comprar_carne) completada. Insumo: OK:comprar_carne
[INICIO] Actividad 4 (asar_longaniza) corriendo con PID 325 [Tiempo: 800ms]
...
```

La actividad 4 solo empieza cuando terminaron la 1 y la 2, y nunca hay más de dos actividades corriendo a la vez.

## Limitaciones conocidas

- **Rendimiento en planes muy encadenados:** para decidir si una actividad puede lanzarse, el programa busca cada dependencia recorriendo el vector completo. En planes aleatorios de 10000 actividades esto es aceptable (unos 9 segundos con tiempos de 1 ms y K = 16), pero en una cadena larga de actividades dependientes el costo crece mucho (una cadena de 2000 actividades tardó cerca de 1 minuto).
- **Los fallos no se originan solos:** el programa no tiene un mecanismo propio para hacer fallar una actividad, así que el aislamiento de errores solo se activa si un hijo termina con error (por ejemplo, un fallo en las tuberías o una señal externa). En fases anteriores, se realizaron pruebas para forzar errores, con el propósito de verificar el manejo correcto de estos, cuyas pruebas resultaron exitosas.
- **Datos de entrada mal formados:** un tiempo no numérico en el archivo o un `K` que no sea un número provoca una excepción de `stol` o `stoi` y el programa termina.
- **Mensajes acotados:** el mensaje de entrada de una actividad crece con la cantidad de dependencias que tenga.

