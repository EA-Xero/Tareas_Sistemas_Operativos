#include <cstdlib>
#include <iostream>
#include <pthread.h>

int max_iter;
int counter = 0;  // Variable compartida global
int lock_var = 0; // 0: libre, 1: ocupado

void *mythread(void *arg) {
  // Spinlock: esperar hasta que esté libre
  while (lock_var)
    ;
  lock_var = 1;

  char *letter = (char *)arg;
  int i; // variable privada en el stack de cada hilo
  std::cout << letter << ": inicio [direccion de i: " << &i << "]" << std::endl;

  for (i = 0; i < max_iter; i++) {
    counter = counter + 1; // sección crítica
  }

  std::cout << letter << ": terminado" << std::endl;

  lock_var = 0; // liberar el spinlock
  return NULL;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "uso: ./spinlock <n_ciclos>" << std::endl;
    exit(1);
  }
  max_iter = atoi(argv[1]);

  pthread_t p1, p2;
  std::cout << "main: inicio [contador = " << counter << "] ["
            << (unsigned long)&counter << "]" << std::endl;

  pthread_create(&p1, NULL, mythread, (void *)"A");
  pthread_create(&p2, NULL, mythread, (void *)"B");

  pthread_join(p1, NULL);
  pthread_join(p2, NULL);

  std::cout << "main: terminado\n [contador: " << counter
            << "]\n [esperado: " << max_iter * 2 << "]" << std::endl;

  return 0;
}
