// write_example.cpp
#include <string.h> // strlen()
#include <unistd.h> // write()

int main() {
  const char *msg = "Hola con write() syscall\n";
  write(1, msg, strlen(msg)); // 1 = STDOUT
  return 0;
}
