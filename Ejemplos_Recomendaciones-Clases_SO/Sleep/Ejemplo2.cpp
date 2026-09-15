#include <iostream>
#include <unistd.h> // usleep

int main() {
  std::cout << "Esperando 500 milisegundos..." << std::endl;
  usleep(500000); // 500,000 µs = 0.5 segundos
  std::cout << "Continuamos" << std::endl;
}
