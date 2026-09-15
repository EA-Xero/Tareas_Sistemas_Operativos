# Sleep
Uso de sleep en C++

## Ejemplos de sleep en C++
La llamada al sistema sleep suspende la ejecución del proceso durante un número de segundos. Es útil para sincronizar tareas o simular tiempos de espera. También existen usleep (microsegundos) y nanosleep (nanosegundos). 

### 1. Ejemplo básico con sleep()
Qué hace: pausa el proceso durante N segundos.

### 2. Ejemplo con usleep()
Qué hace: pausa el proceso en microsegundos (1 segundo = 1,000,000 µs).

### 3. Ejemplo con nanosleep()
Qué hace: permite pausas de mayor precisión con timespec.

# Comparación rápida
´´´
  sleep(segundos) → precisión en segundos.
  usleep(microsegundos) → precisión en microsegundos (obsoleta en POSIX, pero aún común).
  nanosleep(timespec) → precisión en nanosegundos, la más moderna y portable.
´´´

# Salida típica
"
Inicio del programa
Durmiendo 3 segundos...
... (pausa real de 3s) ...
Fin del programa
"
