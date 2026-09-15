#include <iostream>
#include <sys/wait.h> // wait
#include <unistd.h>   // fork, execlp

int main() {
  pid_t pid = fork();

  if (pid == -1) {
    std::cerr << "Error al crear el proceso hijo" << std::endl;
    return 1;
  } else if (pid == 0) {
    // Proceso hijo
    std::cout << "Soy el hijo, voy a ejecutar 'ls -l'..." << std::endl;
    execlp("ls", "ls", "-l", (char *)NULL);
    std::cerr << "Error al ejecutar exec" << std::endl;
    return 1;
  } else {
    // Proceso padre
    std::cout << "Soy el padre, esperando al hijo..." << std::endl;
    int status;
    waitpid(pid, &status, 0);
    std::cout << "El hijo terminó. Código de salida: " << status << std::endl;
  }

  return 0;
}
