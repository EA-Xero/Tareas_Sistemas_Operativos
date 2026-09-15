import os


def ejemplo_waitpid_bloqueante():
    pid = os.fork()
    if pid == 0:
        # Hijo: reemplazar por "echo" (exec)
        os.execlp("echo", "echo", "Hola desde exec()")
        os._exit(127)  # solo si exec falla
    else:
        print(f"Padre: esperando específicamente pid={pid} con waitpid() ...")
        ended_pid, status = os.waitpid(pid, 0)  # bloqueante
        if os.WIFEXITED(status):
            print(f"Padre: hijo {ended_pid} salió con code={os.WEXITSTATUS(status)}")
        elif os.WIFSIGNALED(status):
            print(f"Padre: hijo {ended_pid} terminó por señal {os.WTERMSIG(status)}")


if __name__ == "__main__":
    ejemplo_waitpid_bloqueante()
