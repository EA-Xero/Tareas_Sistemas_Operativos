// pipes.cpp
#include <iostream>
#include <unistd.h>   // para pipe(), fork(), read(), write(), close()
#include <sys/wait.h> // para wait()
#include <cstring>    // para strlen()

int main() {
    int fd[2]; // fd[0] para lectura, fd[1] para escritura
    pid_t pid;
    char buffer[100];

    // Crear el pipe
    if (pipe(fd) == -1) {
        perror("pipe");
        return 1;
    }

    // Crear un proceso hijo
    pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        // Proceso hijo: lee del pipe
        close(fd[1]); // cerrar el extremo de escritura
        int nbytes = read(fd[0], buffer, sizeof(buffer));
        if (nbytes > 0) {
            buffer[nbytes] = '\0'; // asegurar terminación de string
            std::cout << "Hijo recibió: " << buffer << std::endl;
        }
        close(fd[0]);
    } else {
        // Proceso padre: escribe en el pipe
        close(fd[0]); // cerrar el extremo de lectura
        const char* msg = "Hola desde el padre";
        write(fd[1], msg, strlen(msg));
        close(fd[1]);
        wait(NULL); // esperar al hijo
    }

    return 0;
}
