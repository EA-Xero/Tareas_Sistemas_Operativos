// file_syscalls.cpp
#include <fcntl.h>  // open()
#include <stdio.h>  // perror()
#include <stdlib.h> // exit()
#include <string.h> // strlen()
#include <unistd.h> // read(), write(), close()

int main() {
  int fd = open("ejemplo.txt", O_RDONLY);
  if (fd == -1) {
    perror("open");
    exit(1);
  }

  char buffer[128];
  int nbytes = read(fd, buffer, sizeof(buffer) - 1);
  if (nbytes > 0) {
    buffer[nbytes] = '\0';
    write(1, buffer, strlen(buffer)); // mostrar en STDOUT
  }

  close(fd);
  return 0;
}
