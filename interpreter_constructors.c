#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "interpreter.h"
#include "C.tab.h"
#include "stack.h"
#define BOOL_OP 285
/*
 * Constructors for 
 * VALUE, 
 * BINDING, 
 * FRAME, 
 * CLOSURE, 
 * ENV 
 * 
*/ 

VALUE *value_create(int type, void *value) {
    VALUE *new_value = calloc(1,sizeof(VALUE));
    new_value->type = type;
    switch (type) {
        case CONSTANT:
        new_value->v.integer = (int)value;
        break;
        case BOOL_OP:
        new_value->v.boolean = (int)value;
        break;
        case STRING_LITERAL:
        new_value->v.string = (char *)value;
        break;
        case FUNCTION:
        new_value->v.closure = (CLOSURE *)value;
        break;
    }
    return new_value;
}

BINDING *binding_create(TOKEN *name, VALUE *val, BINDING *next) {
    BINDING *new_binding = malloc(sizeof(BINDING));
    new_binding->name = name;
    new_binding->val = val;
    new_binding->next = next;
    return new_binding;
}

FRAME *frame_create(BINDING *bindings) {
    FRAME *new_frame = malloc(sizeof(FRAME));
    new_frame->bindings = bindings;
    return new_frame;
}

CLOSURE *closure_create(NODE *params, NODE *code, FRAME *env) {
    CLOSURE *new_closure = malloc(sizeof(CLOSURE));
    new_closure->params = params;
    new_closure->code = code;
    new_closure->env = env;
    return new_closure;
}

ENV *env_create(STACK *stack, FRAME *global) {
    ENV *new_env = malloc(sizeof(ENV));
    new_env->stack = stack;
    new_env->global = global;
    return new_env;
}

