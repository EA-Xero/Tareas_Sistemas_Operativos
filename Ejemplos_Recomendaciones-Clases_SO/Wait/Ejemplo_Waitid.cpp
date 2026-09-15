#include <iostream>
#include <signal.h>   // siginfo_t
#include <sys/wait.h> // waitid
#include <unistd.h>   // fork, execlp

int main() {
  pid_t pid = fork();
  if (pid == -1) {
    perror("fork");
    return 1;
  }

  if (pid == 0) {
    // Hijo: ejecuta "echo"
    execlp("echo", "echo", "Hola mundo", (char *)NULL);
    perror("exec");
    return 127;
  }

  siginfo_t info{};
  std::cout << "Padre: usando waitid(P_PID, pid, &info, WEXITED)...\n";
  // WEXITED: esperar a que el hijo termine; también existen WSTOPPED,
  // WCONTINUED, WNOHANG
  if (waitid(P_PID, pid, &info, WEXITED) == -1) {
    perror("waitid");
    return 1;
  }

  // info.si_pid = PID del hijo; info.si_status = exit status o señal;
  // info.si_code = causa
  std::cout << "Terminó hijo PID=" << info.si_pid
            << ", si_code=" << info.si_code << ", si_status=" << info.si_status
            << "\n";
}
