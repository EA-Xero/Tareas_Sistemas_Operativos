# clase 8 Peterson (turn)

El algoritmo de Peterson es una solución clásica al problema de la exclusión mutua en sistemas concurrentes. Permite que dos procesos o hilos compartan una sección crítica sin necesidad de primitivas especiales del hardware, garantizando:

  * Exclusión mutua: solo un hilo puede entrar a la sección crítica a la vez.
  * Progreso: si un hilo quiere entrar y el otro no, puede hacerlo sin bloqueos innecesarios.
  * Espera limitada: ningún hilo se queda esperando indefinidamente.

## Explicación del código

  * flag[2]: indica si el hilo quiere entrar a la sección crítica.
  * turn: da prioridad al otro hilo en caso de conflicto.
  * while(flag[other] && turn == other): condición de espera del algoritmo de Peterson.
  * counter: variable compartida que ambos hilos incrementan sin condición de carrera gracias al algoritmo.

## Compilación y ejecución

g++ peterson.cpp -o peterson -pthread
./peterson 100000

El resultado debe mostrar que el valor final de counter coincide con el esperado (max * 2), demostrando que no hubo condiciones de carrera.

