# Python en UNIX: wait, waitpid, waitid y sleep

En sistemas tipo UNIX/Linux, Python expone las llamadas al sistema para gestionar procesos: os.fork() crea un hijo, os.exec* reemplaza el código del proceso, y las funciones os.wait(), os.waitpid() y os.waitid() permiten que el padre espere a sus hijos y lea cómo terminaron. Además, time.sleep() pausa la ejecución.
Requisitos: Ejemplos válidos en UNIX/Linux (no Windows). Python 3.8+; para waitid se recomienda 3.9+

## 1) os.wait() + time.sleep(): esperar a cualquier hijo
Qué hace: os.wait() se bloquea hasta que termine cualquier proceso hijo y devuelve una tupla (pid, status).
Interpretación de status: Usa os.WIFEXITED/os.WEXITSTATUS para salida normal, y os.WIFSIGNALED/os.WTERMSIG si terminó por señal. 

## 2) os.waitpid() + os.exec*: esperar a un hijo específico
Qué hace: os.waitpid(pid, options) espera a un hijo concreto (o a cualquiera con pid=-1). Permite opciones como os.WNOHANG para no bloquear.

Se divide en dos:
* Version A (bloqueante).
* Version B (No bloqueante [Polling]).

## 3) os.waitid(): información detallada (siginfo_t)
Qué hace: os.waitid(idtype, id, options) espera por un hijo (por PID, grupo o cualquiera) y retorna un objeto con campos similares a siginfo_t (p. ej., si_pid, si_code, si_status). Es ideal cuando necesitas diagnosticar la causa de terminación con más precisión. 

* Campos útiles en waitid:
´´´
    si_pid: PID del hijo que cambió de estado.
    si_code: causa (p. ej., CLD_EXITED, CLD_KILLED, CLD_DUMPED, CLD_STOPPED, CLD_CONTINUED).
    si_status: código de salida si CLD_EXITED, o número de señal si terminó por señal.
´´´

# Qué usar en cada caso
´´´
  Un solo hijo, simple: os.wait().
  Varios hijos, ID específico o no bloquear: os.waitpid() (+ os.WNOHANG si necesitas continuar ejecutando).
  Diagnóstico detallado de causa: os.waitid().
´´´
