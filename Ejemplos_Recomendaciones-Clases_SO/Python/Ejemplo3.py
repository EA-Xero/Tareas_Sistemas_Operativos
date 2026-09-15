import os
import time


def ejemplo_waitid_detallado():
    pid = os.fork()
    if pid == 0:
        # Hijo: espera un poco y sale con código 7
        time.sleep(1.0)
        os._exit(7)
    else:
        print(f"Padre: usando waitid(P_PID, {pid}, WEXITED) ...")
        info = os.waitid(os.P_PID, pid, os.WEXITED)  # bloqueante
        # 'info' es un objeto tipo 'os.waitid_result' con atributos estilo siginfo_t
        print(
            f"si_pid={info.si_pid}, si_code={info.si_code}, si_status={info.si_status}"
        )
        # Opcional: si el sistema expone constantes CLD_*
        CLD_EXITED = getattr(os, "CLD_EXITED", None)
        if CLD_EXITED is not None and info.si_code == CLD_EXITED:
            print(f"Terminó normalmente con exit code={info.si_status}")


if __name__ == "__main__":
    ejemplo_waitid_detallado()
