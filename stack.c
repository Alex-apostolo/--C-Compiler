#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "interpreter.h"
#include "stack.h"

STACK* createStack(unsigned capacity)
{
	STACK* stack = calloc(1,sizeof(STACK));
	stack->capacity = capacity;
	stack->top = -1;
	stack->array = calloc(1,stack->capacity * sizeof(FRAME));
	return stack;
}

int isEmpty(STACK* stack)
{
	return stack->top == -1;
}

int isFull(STACK* stack)
{
	return stack->top == stack->capacity - 1;
}

void push(STACK* stack, FRAME *item)
{
	if (isFull(stack))
		return;
	stack->array[++stack->top] = item;
}

FRAME *pop(STACK* stack)
{
	if (isEmpty(stack))
		return NULL;
	return stack->array[stack->top--];
}

FRAME *peek(STACK* stack)
{
	if (isEmpty(stack))
		return NULL;
	return stack->array[stack->top];
}