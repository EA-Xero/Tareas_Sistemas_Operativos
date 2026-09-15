# wait vs waitpid vs waitid en C++

En UNIX/Linux, la “familia wait” permite que un proceso padre espere a sus hijos y obtenga cómo terminaron (código de salida, señal, etc.). Aquí verás: wait (simple), waitpid (controlado) y waitid (detallado).

| Llamada | ¿Qué hace? | ¿Cuándo usar? |	Información disponible |
| :---: | :---: | :---: | :---: |
|wait(int *status) |	Bloquea hasta que termine cualquier hijo y devuelve su PID. |	Cuando no importa qué hijo termine primero. 	|Macros WIFEXITED/… sobre status. |
|waitpid(pid_t pid, int *status, int options) |	Espera a un hijo específico (o a cualquiera) y permite opciones como WNOHANG. |	Cuando necesitas el PID concreto o no quieres bloquear. |	Igual que wait con status; control extra por options.|
|waitid(idtype_t, id_t, siginfo_t*, int) |	Espera por PID, grupo o cualquiera, y llena siginfo_t con datos detallados. |	Cuando necesitas más detalle (p. ej., causa exacta de terminación). |	siginfo_t: si_pid, si_code, si_status, etc. |

* Terminología clave: “Esperar”
el padre se bloquea hasta que el hijo cambia de estado (normalmente, termina). Tras esperar, puedes leer cómo terminó (salida normal, por señal, detenido/continuado).

## 1) Ejemplo con wait() — simple, cualquier hijo
Qué hace: se bloquea hasta que termine cualquier hijo. Devuelve el PID del que terminó y llena status.

## 2) Ejemplo con waitpid() — control por PID y opciones
Qué hace: permite esperar un hijo concreto (pid) o cualquiera (-1), y pasar options como WNOHANG (no bloquear).

## 3) Ejemplo con waitid() — información detallada
Qué hace: espera por tipo/ID (p. ej. P_PID, P_ALL), no devuelve PID sino 0 en éxito y rellena siginfo_t con muchos detalles.

# Cómo interpretar el resultado

* Con wait/waitpid:
´´´
        WIFEXITED(status) → terminó normalmente. Código: WEXITSTATUS(status).
        WIFSIGNALED(status) → terminó por señal. Señal: WTERMSIG(status).
        WIFSTOPPED(status), WSTOPSIG(status) → fue detenido por señal.
        WIFCONTINUED(status) → fue reanudado (si se usa WCONTINUED).
´´´
* Con waitid:
´´´
        info.si_pid → PID del hijo que cambió de estado.
        info.si_code → causa (p. ej., CLD_EXITED, CLD_KILLED, CLD_DUMPED, CLD_STOPPED, CLD_CONTINUED).
        info.si_status → código de salida si CLD_EXITED, o número de señal si fue por señal
´´´
