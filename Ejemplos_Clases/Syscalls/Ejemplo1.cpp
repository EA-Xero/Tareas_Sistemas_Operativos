// fork_getpid.cpp
#include <unistd.h>   // fork(), getpid(), getppid()
#include <sys/wait.h> // wait()
#include <iostream>

int main() {
    pid_t pid = fork();

    if (pid == 0) {
        // Proceso hijo
        std::cout << "Soy el hijo, PID: " << getpid()
                  << ", Padre: " << getppid() << std::endl;
    } else if (pid > 0) {
        // Proceso padre
        std::cout << "Soy el padre, PID: " << getpid() << std::endl;
        wait(NULL);
    } else {
        perror("fork");
    }

    return 0;
}
