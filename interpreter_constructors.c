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
 * structs
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

VALUE *find_ident_value(TOKEN *t, FRAME *frame, ENV *env) {
    BINDING *bindings = frame->bindings;
    while (bindings != NULL) {
        if (strcmp(bindings->name->lexeme, t->lexeme) == 0) {
            return bindings->val;
        }
        bindings = bindings->next;
    }

    if (env != NULL) {
        BINDING *bindings = env->global->bindings;
        while (bindings != NULL) {
            if (strcmp(bindings->name->lexeme, t->lexeme) == 0) {
                return bindings->val;
            }
            bindings = bindings->next;
        }
    }
    // error(" unbound variable ");
}

/**
 * Helper functions for environment
 * Declare binding
 * Assing binding
 * Extend Frame
 * Print Frame
 * 
**/

void declare(TOKEN *identifier, FRAME *frame) {
    BINDING *temp = frame->bindings;
    BINDING *new_bind = binding_create(identifier, NULL, NULL);

    if (new_bind != 0) {
        if (temp == NULL) {
            frame->bindings = new_bind;
            return;
        }
        while (temp->next != NULL)
            temp = temp->next;
        temp->next = new_bind;
    }
    // error (" binding allocation failed " );
}

VALUE *assing_value(TOKEN *t, FRAME *frame, VALUE *value, ENV *env) {
    // if its an identifier then take its value
    // Search the frame provided and then global
    if (frame != NULL) {
        BINDING *bindings = frame->bindings;
        while (bindings != NULL) {
            if (strcmp(bindings->name->lexeme, t->lexeme) == 0) {
                if (value->type == IDENTIFIER) {
                    TOKEN *token = new_token(IDENTIFIER);
                    token->lexeme = value->v.string;
                    bindings->val = find_ident_value(token, frame, env);
                } else
                    bindings->val = value;
                return bindings->val;
            }
            bindings = bindings->next;
        }
    }
    if (env != NULL) {
        BINDING *bindings = env->global->bindings;
        while (bindings != NULL) {
            if (strcmp(bindings->name->lexeme, t->lexeme) == 0) {
                if (value->type == IDENTIFIER) {
                    TOKEN *token = new_token(IDENTIFIER);
                    token->lexeme = value->v.string;
                    bindings->val = find_ident_value(token, frame, env);
                } else
                    bindings->val = value;
                return bindings->val;
            }
            bindings = bindings->next;
        }
    }
    return NULL;
    // error(" unbound variable");
}

// Responsible for extending the frame, used when we call a function
FRAME *extend_frame(FRAME *frame) {
    // Function body makes a deep copy of the supplied frame
    FRAME *newenv = frame_create(NULL);
    BINDING *temp = frame->bindings;

    while (temp != NULL) {
        // Copy name of binding and declare it in newenv
        TOKEN *new_name = new_token(IDENTIFIER);
        char *temp_name = malloc(strlen(temp->name->lexeme) + 1);
        strcpy(temp_name, temp->name->lexeme);
        new_name->lexeme = temp_name;
        declare(new_name, newenv);

        // Copy value of binding and assing it in newenv
        VALUE *new_value;
        if (temp->val != NULL) {
            switch (temp->val->type) {
            case CONSTANT:
                new_value =
                    value_create(CONSTANT, (void *)temp->val->v.integer);
                assing_value(new_name, newenv, new_value, NULL);
                break;
            case STRING_LITERAL:
                new_value =
                    value_create(STRING_LITERAL, (void *)temp->val->v.string);
                assing_value(new_name, newenv, new_value, NULL);
                break;
            case FUNCTION: {
                CLOSURE *new_closure =
                    closure_create(temp->val->v.closure->params,
                                   temp->val->v.closure->code, newenv);
                new_value = value_create(FUNCTION, (void *)new_closure);
                assing_value(new_name, newenv, new_value, NULL);
                break;
            }
            }
        }
        temp = temp->next;
    }
    return newenv;
}

void print_bindings(FRAME *frame) {
    // Make this print every frame in the environment
    printf("*** BINDING LIST ***\n\n");
    BINDING *temp = frame->bindings;
    while (temp != NULL) {
        printf("%s\n", temp->name->lexeme);
        temp = temp->next;
    }
    printf("\n*** END OF BINDING LIST ***\n\n\n");
}
