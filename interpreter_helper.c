#include <stdlib.h>
#include "interpreter.h"
#include "C.tab.h"

/**
 * Helper functions for environment
 * Declare binding
 * Assing binding
 * Extend Frame
 * Print Frame
 * 
**/

VALUE *find_ident_value(TOKEN *, FRAME *, ENV *);

// Declare identifier in frame, append to the end of BINDINGS
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

// Assing value to an identifier in the environment
VALUE *assing_value(TOKEN *identifier, VALUE *value, FRAME *frame, ENV *env) {
    // Search the frame provided for the identifier
    if (frame != NULL) {
        BINDING *bindings = frame->bindings;
        while (bindings != NULL) {
            if (strcmp(bindings->name->lexeme, identifier->lexeme) == 0) {
                // When identifier is found create a binding
                if (value->type == IDENTIFIER) {
                    // Case where x = y is handled here
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
    // If it's not found search the global frame in the same way
    if (env != NULL) {
        BINDING *bindings = env->global->bindings;
        while (bindings != NULL) {
            if (strcmp(bindings->name->lexeme, identifier->lexeme) == 0) {
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

// Find the value of an identifier in the environment
VALUE *find_ident_value(TOKEN *identifier, FRAME *frame, ENV *env) {
    // Search the frame provided for the identifier
    BINDING *bindings = frame->bindings;
    while (bindings != NULL) {
        if (strcmp(bindings->name->lexeme, identifier->lexeme) == 0) {
            // Return idents value when found
            return bindings->val;
        }
        bindings = bindings->next;
    }

    // If it's not found search the global frame in the same way
    if (env != NULL) {
        BINDING *bindings = env->global->bindings;
        while (bindings != NULL) {
            if (strcmp(bindings->name->lexeme, identifier->lexeme) == 0) {
                return bindings->val;
            }
            bindings = bindings->next;
        }
    }
    // error(" unbound variable ");
}

// Responsible for extending the frame, used when we call a function
// Function makes a DEEP COPY of the supplied frame
FRAME *extend_frame(FRAME *frame) {
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
                assing_value(new_name, new_value, newenv, NULL);
                break;
            case STRING_LITERAL:
                new_value =
                    value_create(STRING_LITERAL, (void *)temp->val->v.string);
                assing_value(new_name, new_value, newenv, NULL);
                break;
            case FUNCTION: {
                CLOSURE *new_closure =
                    closure_create(temp->val->v.closure->params,
                                   temp->val->v.closure->code, newenv);
                new_value = value_create(FUNCTION, (void *)new_closure);
                assing_value(new_name, new_value, newenv, NULL);
                break;
            }
            }
        }
        temp = temp->next;
    }
    return newenv;
}

// Helper method for looking at the global bindings
// Use -b flag to invoke this
void print_bindings(FRAME *frame) {
    printf("*** BINDING LIST ***\n\n");
    BINDING *temp = frame->bindings;
    while (temp != NULL) {
        printf("%s\n", temp->name->lexeme);
        temp = temp->next;
    }
    printf("\n*** END OF BINDING LIST ***\n\n\n");
}