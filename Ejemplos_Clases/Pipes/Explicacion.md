# Pipes.gcc
Ejemplo de uso de Pipes en C++

' Los pipes en C++ permiten la comunicación entre procesos. Un proceso puede escribir en un pipe y otro proceso puede leer desde él. Este mecanismo se usa frecuentemente entre procesos padre e hijo creados con fork().'

## Compilación y ejecución

* Para compilar y ejecutar este ejemplo:
´´´g++ pipes.cpp -o pipes
./pipes
´´´
## Salida esperada

* Hijo recibió: Hola desde el padre
