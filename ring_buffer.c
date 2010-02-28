#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>

#define DIGITS 10

int size = 0;
int start = 0;
int end = 0;

void push(int x, int p, int fd) {
  char x_string[DIGITS + 1];
  char p_string[DIGITS + 1];

  snprintf(x_string, DIGITS + 1, "%010d", x);
  snprintf(p_string, DIGITS + 1, "%010d", p);

  lseek(fd, end, SEEK_SET);

  write(fd, x_string, DIGITS + 1);
  write(fd, p_string, DIGITS + 1);
  size++;
  end += 2 * (DIGITS + 1);
}

void pop(int fd, char* data) {
  read(fd, data, 2 * (DIGITS + 1));
  start += 2 * (DIGITS + 1);
}

int main() 
{

}

