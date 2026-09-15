#include <iostream>
#include <unistd.h> // sleep

int main() {
  std::cout << "Inicio del programa" << std::endl;

  std::cout << "Durmiendo 3 segundos..." << std::endl;
  sleep(3); // pausa 3 segundos

  std::cout << "Fin del programa" << std::endl;
  return 0;
}
