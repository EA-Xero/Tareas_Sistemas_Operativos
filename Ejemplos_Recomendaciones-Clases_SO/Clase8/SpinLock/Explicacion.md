# clase 8 ejemplo Spinlock

Este ejemplo implementa un spinlock básico en C++ usando una variable global como mecanismo de exclusión mutua. Un spinlock consiste en un bucle activo (while) donde un hilo espera hasta que el recurso compartido (en este caso, la sección crítica) quede libre.

Se crean dos hilos (A y B), ambos incrementan la variable global counter. El spinlock asegura que solo un hilo a la vez ejecute la sección crítica.

## Explicación del código

  * lock_var: funciona como candado. Si vale 0, la sección crítica está libre. Si vale 1, significa que otro hilo está dentro.
  * while(lock_var);: bucle de espera activa, los hilos se quedan "girando" hasta que el recurso esté disponible.
  * counter: variable global compartida que los hilos incrementan. Gracias al spinlock, no hay condición de carrera.
  * pthread_create: crea los hilos A y B.
  * pthread_join: asegura que el programa principal espere a que los hilos terminen.

## Compilación y ejecución

* Para compilar en Linux/Unix:
g++ spinlock.cpp -o spinlock -pthread

* Ejemplo de ejecución con 100000 ciclos:
./spinlock 100000

El resultado final debería coincidir con el valor esperado (max * 2) ya que el spinlock evita la condición de carrera.
