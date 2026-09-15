#include <iostream>
#include <sys/wait.h> // wait, macros WIF*
#include <unistd.h>   // fork, execlp

int main() {
  pid_t pid = fork();
  if (pid == -1) {
    perror("fork");
    return 1;
  }

  if (pid == 0) {
    // Hijo: ejecuta "date"
    execlp("date", "date", (char *)NULL);
    perror("exec");
    return 127; // solo si exec falla
  }

  // Padre: espera a CUALQUIER hijo
  int status = 0;
  pid_t ended = wait(&status);
  if (ended == -1) {
    perror("wait");
    return 1;
  }

  std::cout << "Padre: terminó el hijo PID=" << ended << "\n";
  if (WIFEXITED(status)) {
    std::cout << "Salida normal, code=" << WEXITSTATUS(status) << "\n";
  } else if (WIFSIGNALED(status)) {
    std::cout << "Terminó por señal " << WTERMSIG(status) << "\n";
  }
}
