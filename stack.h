#ifndef __STACK_H
#define __STACK_H
#include "interpreter.h"

typedef struct stack {
	int top;
	unsigned capacity;
	FRAME **list;
}STACK;

STACK *create_stack(unsigned capacity);
int is_empty(STACK* stack);
void push(STACK* stack, FRAME* item);
FRAME *pop(STACK* stack);
FRAME *peek(STACK* stack);
#endif