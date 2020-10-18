#include <stdio.h>
#include "interpreter.h"
int main() {
  char* x;
  fputc(92,stdin);
  printf("%d",fgetc(stdin));
  return 0;
}