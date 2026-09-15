import os
import time


def ejemplo_wait_y_sleep():
    pid = os.fork()
    if pid == 0:
        # Proceso hijo
        print("Hijo: duermo 2s y salgo con código 42", flush=True)
        time.sleep(2)
        os._exit(42)  # Termina el hijo con exit code 42
    else:
        # Proceso padre
        print("Padre: esperando a CUALQUIER hijo con wait() ...")
        ended_pid, status = os.wait()  # bloqueante
        if os.WIFEXITED(status):
            code = os.WEXITSTATUS(status)
            print(f"Padre: terminó hijo pid={ended_pid}, exit={code}")
        elif os.WIFSIGNALED(status):
            print(
                f"Padre: hijo pid={ended_pid} terminó por señal {os.WTERMSIG(status)}"
            )


if __name__ == "__main__":
    ejemplo_wait_y_sleep()
