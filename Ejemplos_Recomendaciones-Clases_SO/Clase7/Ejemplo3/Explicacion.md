# Clase 7 ejemplo 3

Este programa demuestra la interacción entre procesos (creados con fork()) e hilos (creados con pthread_create) en C++. Cada proceso (padre e hijo) crea un hilo que modifica la variable global sharedVar.

Lo importante es que después de un fork(), la memoria entre padre e hijo no es compartida: cada proceso mantiene su propia copia de sharedVar. Sin embargo, dentro de un mismo proceso los hilos sí comparten memoria.

## Explicación del código

* fork(): crea un proceso hijo idéntico al padre, pero con su propio espacio de memoria.
* sharedVar: es global y compartida entre los hilos, pero no entre procesos (padre e hijo tienen copias independientes).
* pthread_create: cada proceso crea un hilo que modifica sharedVar.
* sched_yield(): cede el control al planificador, permitiendo que otros hilos o procesos se ejecuten.
* pthread_join: asegura que el proceso espere a que el hilo termine antes de continuar.

## Compilación y ejecución

* Para compilar el programa en Linux/Unix:
g++ clase7_ejemplo3.cpp -o clase7_ejemplo3 -pthread

Al ejecutarlo, verás que el proceso padre y el hijo muestran sharedVar, pero los cambios de un proceso no afectan al otro.

