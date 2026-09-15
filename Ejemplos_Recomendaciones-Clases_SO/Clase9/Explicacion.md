# Clase 9 Ejemplos de Mutex Bueno y Malo
Uso correcto e incorrecto de Mutex en C++

En C++ podemos trabajar con hilos usando std::thread y proteger recursos compartidos con std::mutex. Al igual que en C con Pthreads, si no usamos un mutex tendremos condiciones de carrera.

❌ Ejemplo incorrecto: mutex_mal.cpp
Aquí lanzamos dos hilos que modifican la variable compartida counter sin sincronización. El resultado es impredecible.

✅ Ejemplo correcto: mutex_bien.cpp
En este ejemplo usamos std::mutex para proteger la sección crítica. Ahora solo un hilo puede entrar a modificar counter a la vez.

# Conclusión
Usar std::mutex en C++ nos permite controlar el acceso a los recursos compartidos y evitar condiciones de carrera. El ejemplo sin mutex es rápido pero inseguro, mientras que con mutex garantizamos la correcta sincronización de los hilos.
