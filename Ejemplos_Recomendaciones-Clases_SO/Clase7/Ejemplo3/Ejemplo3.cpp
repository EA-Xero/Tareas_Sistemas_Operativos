#include <cstdlib>
#include <iostream>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>

volatile int sharedVar = 1234; // Variable global

void *mythread(void *arg) {
  std::cout << "\n thread " << pthread_self() << ": sharedVar ahora vale "
            << sharedVar << std::endl;
  sharedVar = 111;
  std::cout << "\n thread " << pthread_self() << ": sharedVar ahora vale "
            << sharedVar << std::endl;
  sched_yield();
  std::cout << "\n thread " << pthread_self() << ": sharedVar ahora vale "
            << sharedVar << std::endl;
  sharedVar = 222;
  std::cout << "\n thread " << pthread_self() << ": sharedVar ahora vale "
            << sharedVar << std::endl;
  sched_yield();
  pthread_exit(NULL);
}

int main() {
  pthread_t tid;
  int pid;

  std::cout << "\n Main: sharedVar = " << sharedVar << std::endl;

  pid = fork(); // se crea proceso hijo

  if (pid == 0) { // Proceso hijo
    pthread_create(&tid, NULL, mythread, NULL);
    std::cout << "\n Hijo: creo thread " << tid << ". sharedVar = " << sharedVar
              << std::endl;
    sched_yield();
    std::cout << "\n Hijo: sharedVar = " << sharedVar << std::endl;
    sched_yield();
    std::cout << "\n Hijo: sharedVar = " << sharedVar << std::endl;
    pthread_join(tid, NULL);
    std::cout << "Hijo: Terminado" << std::endl;
  }

  if (pid > 0) { // Proceso padre
    pthread_create(&tid, NULL, mythread, NULL);
    std::cout << "\n Padre: creo thread " << tid
              << ". sharedVar = " << sharedVar << std::endl;
    std::cout << "\n Padre: sharedVar = " << sharedVar << std::endl;
    sharedVar = 44444444;
    std::cout << "\n Padre: sharedVar = " << sharedVar << std::endl;
    sched_yield();
    std::cout << "\n Padre: sharedVar = " << sharedVar << std::endl;
    sched_yield();
    std::cout << "\n Padre: sharedVar = " << sharedVar << std::endl;
  }

  exit(0);
}
