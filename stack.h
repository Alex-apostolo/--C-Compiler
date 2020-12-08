#ifndef __STACK_H
#define __STACK_H
#include "interpreter.h"

typedef struct stack {
	int top;
	unsigned capacity;
	FRAME **array;
}STACK;

STACK *createStack(unsigned capacity);
int isFull(STACK* stack);
int isEmpty(STACK* stack);
void push(STACK* stack, FRAME* item);
FRAME *pop(STACK* stack);
FRAME *peek(STACK* stack);
#endif