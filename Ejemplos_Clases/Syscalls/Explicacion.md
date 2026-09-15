# syscalls

Ejemplos de Syscalls en C/C++

Las llamadas al sistema (syscalls) permiten que los programas interactúen con el kernel de Linux/Unix. A través de ellas se pueden realizar operaciones de bajo nivel como manejo de procesos, archivos o comunicación entre procesos.

## Ejemplo 1: Uso de fork() y getpid()
Este ejemplo crea un proceso hijo y muestra los PID de padre e hijo

## Ejemplo 2: Escritura con write()
En lugar de printf o std::cout, podemos escribir directamente en la salida estándar

## Ejemplo 3: Manejo de archivos con open(), read() y close()
Este programa abre un archivo de texto, lee su contenido y lo muestra en pantalla:

# Compilación y ejecución

´´´g++ fork_getpid.cpp -o fork_getpid
./fork_getpid

g++ write_example.cpp -o write_example
./write_example

g++ file_syscalls.cpp -o file_syscalls
./file_syscalls
´´´
