# clase7_ejemplo2.cpp

Este ejemplo muestra cómo dos hilos (A y B) acceden a una variable global compartida (counter). Ambos incrementan esta variable en paralelo, lo que puede provocar una condición de carrera, ya que la operación counter = counter + 1 no es atómica.

## Explicación del código

* counter: variable compartida por ambos hilos. Como no tiene protección (mutex, semáforos, etc.), puede producir resultados incorrectos.
* pthread_create: crea los hilos A y B, cada uno ejecuta la función mythread.
* mythread: imprime un mensaje al inicio y al final, e incrementa counter tantas veces como se indicó en la entrada.
* pthread_join: asegura que el programa principal espere a que ambos hilos terminen antes de mostrar el resultado final.
* Condición de carrera: el resultado final de counter podría ser menor a max*2 porque los hilos pueden interferir entre sí al actualizar la variable.

## Compilación y ejecución

* Para compilar este programa en Linux/Unix, se utiliza:
g++ clase7_ejemplo2.cpp -o clase7_ejemplo2 -pthread

* Ejemplo de ejecución con 100000 ciclos:
./clase7_ejemplo2 100000

En este caso, el valor esperado es 200000, pero muchas veces el valor real será menor debido a la condición de carrera.

