# Ejemplo uso de exec en C++
Ejemplo: exec con fork() y wait()

En este ejemplo, el proceso padre crea un hijo con fork(). El hijo reemplaza su código con ls -l usando exec, mientras que el padre espera con waitpid() hasta que el hijo termine.

# Salida esperada
´´´
Soy el padre, esperando al hijo...
Soy el hijo, voy a ejecutar 'ls -l'...
(total de archivos y permisos que devuelve ls -l)
El hijo terminó. Código de salida: 0
´´´
