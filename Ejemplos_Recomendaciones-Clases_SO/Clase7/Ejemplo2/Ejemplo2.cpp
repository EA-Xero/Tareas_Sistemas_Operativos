#include <cstdlib>
#include <iostream>
#include <pthread.h>

int max_iter;
volatile int counter = 0; // Variable compartida global

void *mythread(void *arg) {
  char *letter = (char *)arg;
  int i;
  std::cout << letter << ": inicio [direccion de i: " << &i << "]" << std::endl;
  for (i = 0; i < max_iter; i++) {
    counter = counter + 1; // Operación no atómica
  }
  std::cout << letter << ": terminado" << std::endl;
  return NULL;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "uso: ./pthreads2 <n_ciclos>" << std::endl;
    exit(1);
  }
  max_iter = atoi(argv[1]);

  pthread_t p1, p2;
  std::cout << "main: inicio [contador = " << counter << "] ["
            << (unsigned long)&counter << "]" << std::endl;

  pthread_create(&p1, NULL, mythread, (void *)"A");
  pthread_create(&p2, NULL, mythread, (void *)"B");

  // Esperamos a que los hilos terminen
  pthread_join(p1, NULL);
  pthread_join(p2, NULL);

  std::cout << "main: terminado\n [contador: " << counter
            << "]\n [esperado: " << max_iter * 2 << "]" << std::endl;

  return 0;
}
