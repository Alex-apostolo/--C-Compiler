#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "interpreter.h"
#include "stack.h"

STACK* create_stack(unsigned capacity)
{
	STACK* stack = calloc(1,sizeof(STACK));
	stack->capacity = capacity;
	stack->top = -1;
	stack->list = calloc(1,stack->capacity * sizeof(FRAME));
	return stack;
}

int is_empty(STACK* stack)
{
	if(stack->top == -1) return 1;
	else return 0;
}

void push(STACK* stack, FRAME *item)
{
	int index = ++stack->top;
	stack->list[index] = item;
}

FRAME *pop(STACK* stack)
{
	if (is_empty(stack))
		return NULL;
	return stack->list[stack->top--];
}

FRAME *peek(STACK* stack)
{
	if (is_empty(stack))
		return NULL;
	return stack->list[stack->top];
}