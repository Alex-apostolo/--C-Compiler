#include <stdio.h>
int main() {
  struct lol {
    int i;
    char b;
    struct lol *next;
  };
  printf("troll\n\n");
  struct lol aman = {10,34,NULL};
  struct lol aman2 = {23,55,NULL};
  aman2.next = &aman;
  return 0;
}