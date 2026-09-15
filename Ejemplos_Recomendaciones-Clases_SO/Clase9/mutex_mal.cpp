#include <iostream>
#include <thread>

int counter = 0; // recurso compartido

void funcionThread() {
  counter += 1;
  std::cout << "\n Inicia job " << counter << std::endl;

  for (volatile unsigned long i = 0; i < 0xFFFFFFF; i++)
    ; // simular trabajo

  std::cout << " Termina job " << counter << std::endl;
}

int main() {
  std::thread t1(funcionThread);
  std::thread t2(funcionThread);

  t1.join();
  t2.join();

  return 0;
}
