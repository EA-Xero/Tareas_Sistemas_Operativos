#include <iostream>
#include <mutex>
#include <thread>

int counter = 0; // recurso compartido
std::mutex mtx;  // el mutex

void funcionThread() {
  mtx.lock(); // 🔒 sección crítica

  counter += 1;
  std::cout << "\n Inicia job " << counter << std::endl;

  for (volatile unsigned long i = 0; i < 0xFFFFFFF; i++)
    ; // simular trabajo

  std::cout << " Termina job " << counter << std::endl;

  mtx.unlock(); // 🔓 libera el candado
}

int main() {
  std::thread t1(funcionThread);
  std::thread t2(funcionThread);

  t1.join();
  t2.join();

  return 0;
}
