import os
import time


def ejemplo_waitpid_no_bloqueante():
    pid = os.fork()
    if pid == 0:
        # Hijo: simular trabajo "largo"
        time.sleep(1.5)
        os._exit(0)
    else:
        print(
            f"Padre: consultando sin bloquear con waitpid(WNOHANG) sobre pid={pid} ..."
        )
        while True:
            ended_pid, status = os.waitpid(pid, os.WNOHANG)
            if ended_pid == 0:
                # Aún no terminó
                print("Padre: hijo sigue trabajando; hago otras cosas ...")
                time.sleep(0.2)
                continue
            # Terminó
            if os.WIFEXITED(status):
                print(
                    f"Padre: hijo {ended_pid} terminó con code={os.WEXITSTATUS(status)}"
                )
            else:
                print(f"Padre: hijo {ended_pid} terminó (no por salida normal)")
            break


if __name__ == "__main__":
    ejemplo_waitpid_no_bloqueante()
