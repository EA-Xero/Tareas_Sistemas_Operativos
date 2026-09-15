#include <cstdlib>
#include <iostream>
#include <pthread.h>

int max_iter;
int counter = 0;      // Variable compartida
int turn = 1;         // A quién le toca entrar en la SC
int flag[2] = {0, 0}; // Indica si un hilo quiere entrar

void *mythread1(void *arg) {
  int me = 0;
  int other = 1;
  char *letter = (char *)arg;
  int i;

  flag[me] = 1; // Quiero entrar
  turn = other; // Le cedo el turno al otro
  while (flag[other] && turn == other)
    ; // Espero si no me toca

  std::cout << letter << ": inicio [direccion de i: " << &i << "]" << std::endl;
  for (i = 0; i < max_iter; i++) {
    counter = counter + 1; // sección crítica
  }
  std::cout << letter << ": terminado" << std::endl;

  flag[me] = 0; // Ya no quiero entrar
  return NULL;
}

void *mythread2(void *arg) {
  int me = 1;
  int other = 0;
  char *letter = (char *)arg;
  int i;

  flag[me] = 1;
  turn = other;
  while (flag[other] && turn == other)
    ;

  std::cout << letter << ": inicio [direccion de i: " << &i << "]" << std::endl;
  for (i = 0; i < max_iter; i++) {
    counter = counter + 1;
  }
  std::cout << letter << ": terminado" << std::endl;

  flag[me] = 0;
  return NULL;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "uso: ./turn <n_ciclos>" << std::endl;
    exit(1);
  }
  max_iter = atoi(argv[1]);

  pthread_t p1, p2;
  std::cout << "main: inicio [contador = " << counter << "] ["
            << (unsigned long)&counter << "]" << std::endl;

  pthread_create(&p1, NULL, mythread1, (void *)"A");
  pthread_create(&p2, NULL, mythread2, (void *)"B");

  pthread_join(p1, NULL);
  pthread_join(p2, NULL);

  std::cout << "main: terminado\n [contador: " << counter
            << "]\n [esperado: " << max_iter * 2 << "]" << std::endl;

  return 0;
}
