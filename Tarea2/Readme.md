# Tarea 2 | Kamikan

Eso si, Kamikan distinta distribucion, pero al fin y al cabo el mismo juego

## Compilacion
 - Ubicarse en "Tareas_Sistemas_Operativos/Tarea2" y Escribir "Make" en la terminal, esta debe estar al mismo nivel del archivo "Makefile" presentado en la
 carpeta Tarea 2.

## Output
 - Archivos .out de los modulos se quedaran en la carpeta src
 - Carpeta Juego, para separar de los modulos
 - Archivo del torneo .out quedara en la carpeta Juego
 - Carpeta de Logs, abarcara archivos .log por mesa que tendran informacion relevante sobre las jugadas realizadas

## Iniciar juego
 - Si quieres colocar una configuracion en especifico, debes colocarla como parametro al ejecutar el archivo, recomiendo meterla en la carpeta de "ConfigFiles" para no tener archivos volando por ahi, una vez listo Ejecutar lo siguiente: "./Juego/torneo.out [N.jugadores] [M.Mesas] <config>"
    el archivo config es un selector de dificultad practicamente, te deja colocar especificamente el piso y techo del tiempo de retardo de cada interaccion, posiciones del puma y de los perros y una semilla para que sepas que archivo cargaste o acciones de debug del codigo

## Esperar juego
 - Luego de ejecutar eso, se asignaran pares y mesas, si faltan mesas esos jugadores esperaran y si el numero de jugadores es impar ese se queda sin pareja.
 - el detalle de movimientos, capturas y ganadores se guardaran en la carpeta Logs. Mientras que el tablero se dibujara en la terminal (a pesar de su forma en la terminal, el triangulo si esta conectado exactamente como lo pide la rubrica, pero es dificil dibujarlo exacto en la terminal)

## Termino
 - Al terminar las partidas, el programa finaliza, puedes verificar todo en los logs.

## Limpiza
 - Cuando estes chato de jugar, ya no quieres tantos archivos encima, escribe "make clean", esto limpiara lo siguiente:
    - Logs/
    - Juego/
    - src/*.o
 - Si quieres una limpieza explicita (por ejemplo borrar logs pero no game), puedes hacer lo siguiente:
   - make clean: Ejecuta las tres inferiores al en secuencia
   - make cleanSrc: Borra archivos.o de la carpeta src
   - make cleanLogs: Borra la carpeta Logs y su contenido
   - make cleanGame: Borra la carpeta Juego y su contenido
 - Basicamente te deja con lo mismo que como vino en el git