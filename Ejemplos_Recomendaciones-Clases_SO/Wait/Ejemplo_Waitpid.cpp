#include <iostream>
#include <sys/wait.h> // waitpid, macros
#include <unistd.h>   // fork, execlp

int main() {
  pid_t pid = fork();
  if (pid == -1) {
    perror("fork");
    return 1;
  }

  if (pid == 0) {
    // Hijo: ejecuta "ls -l"
    execlp("ls", "ls", "-l", (char *)NULL);
    perror("exec");
    return 127;
  }

  int status = 0;
  std::cout << "Padre: esperando al hijo PID=" << pid << "...\n";

  // Espera bloqueante al hijo específico
  pid_t ended = waitpid(pid, &status, 0);
  if (ended == -1) {
    perror("waitpid");
    return 1;
  }

  if (WIFEXITED(status)) {
    std::cout << "Hijo salió con code=" << WEXITSTATUS(status) << "\n";
  } else if (WIFSIGNALED(status)) {
    std::cout << "Hijo terminó por señal " << WTERMSIG(status) << "\n";
  }

  // Ejemplo WNOHANG (no bloquea):
  // pid_t r = waitpid(-1, &status, WNOHANG); // consulta sin bloquear
}
