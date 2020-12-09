#include "interpreter.h"
#include "C.tab.h"
#include "stack.h"
#include <stdio.h>

FRAME *extend_frame(FRAME *);
VALUE *__interpret(NODE *, ENV *);
VALUE *find_ident_value(TOKEN *, FRAME *);
VALUE *assing_value(TOKEN *, FRAME *, VALUE *);
void declare(TOKEN *, FRAME *);

/*Function responsible for initializing*/
VALUE *interpret(NODE *term, ENV *env) {
    // Add global frame if it doesn't exist
    if (env->global == NULL)
        env->global = calloc(1, sizeof(FRAME));
    env->stack = createStack(1024 * 1024);
    push(env->stack, env->global);

    // Initiates global frame and saves it to stack
    __interpret(term, env);

    // Pop global frame from Stack and set Stack to NULL
    env->global = pop(env->stack);

    // Search for main and execute it
    int main_exists = 0;
    BINDING *temp = env->global->bindings;
    while (temp != NULL) {
        // strcmp not working
        if (strcmp(temp->name->lexeme, "main") == 0) {
            main_exists = 1;
            // Push main frame to stack
            FRAME *main_frame = extend_frame(env->global);
            push(env->stack, main_frame);
            if (temp->val->v.closure->code == NULL) {
                fprintf(stderr, "Error, no return type for main\n");
                VALUE *val = malloc(sizeof(VALUE));
                val->type = CONSTANT;
                val->v.integer = -1;
                return val;
            }
            return __interpret(temp->val->v.closure->code, env);
        }
        temp = temp->next;
    }
    if (main_exists == 0)
        fprintf(stderr, "Error Main function could not be located\n");
}

VALUE *__interpret(NODE *term, ENV *env) {
    switch (term->type) {
    case LEAF:
        // Left child has value: STRING_LITERAL, IDENTIFIER, CONSTANT
        return __interpret(term->left, env);
        break;
    case IDENTIFIER:
        return term;
        break;
    case CONSTANT: {
        // Term holds its value on the right child
        VALUE *constant = calloc(1, sizeof(VALUE));
        constant->type = CONSTANT;
        constant->v.integer = term->right;
        return constant;
        break;
    }
    case STRING_LITERAL: {
        // Term holds its value on the left child
        VALUE *string = calloc(1, sizeof(VALUE));
        string->type = STRING_LITERAL;
        string->v.string = term->left;
        return string;
    }
    case APPLY:
        // Process calling of functions
        break;
    case VOID:
    case FUNCTION:
    case INT:
        break;
    case 'd':
        // Left child is an AST for Return type
        __interpret(term->left, env);
        // Right child is the Name and arguments
        return __interpret(term->right, env);
        break;
    case 'D': {
        // Creates new closure and assigns it to function defined
        // check if they are fine
        VALUE *closure = calloc(1, sizeof(VALUE));

        CLOSURE *new_closure = calloc(1, sizeof(CLOSURE));
        new_closure->code = term->right;
        new_closure->env = env->stack;

        closure->v.closure = new_closure;
        closure->type = FUNCTION;
        assing_value(__interpret(term->left, env), peek(env->stack), closure);
        break;
    }
    case 'F': {
        // Right child are the Arguments of function
        if (term->right != NULL) {
            __interpret(term->right, env);
        }
        // Left child is the Name of function
        TOKEN *t = __interpret(term->left, env);
        declare(t, peek(env->stack));
        return t;
        break;
    }
    case CONTINUE:
    case BREAK:
        break;
    case RETURN:
        // Left child is an AST of the expression whose value is to be returned
        // TODO: handle the case where left child is identifier and the case for
        // everything else
        if (term->left != NULL) {
            if (term->left->type == LEAF &&
                term->left->left->type == IDENTIFIER) {
                return find_ident_value(__interpret(term->left, env),
                                        peek(env->stack));
            } else
                return __interpret(term->left, env);
        } else {
            // TODO: throw an error
            printf("Error no return type\n");
        }
        break;
    case '~':
        if (term->left->type == LEAF) {
            if (term->right->type == LEAF) {
                // Right child is the variable name
                declare(__interpret(term->right, env), peek(env->stack));
            } else {
                // Right child is the AST "=" and we declare the variable before
                // assigning
                declare(__interpret(term->right->left, env), peek(env->stack));
                __interpret(term->right, env);
            }
        } else {
            __interpret(term->left, env);
            __interpret(term->right, env);
        }
        break;
    case ';':
        // Left child is the first item or a sequence; returned values are
        // ignored
        __interpret(term->left, env);
        // Right child is the last item and also the return value
        return __interpret(term->right, env);
        break;
    case '=':
        assing_value(__interpret(term->left, env), peek(env->stack),
                     __interpret(term->right, env));
        break;
    case '+':
    case '-':
    case '*':
    case '/':
    case '%':
    case '>':
    case '<':
    case NE_OP:
    case EQ_OP:
    case LE_OP:
    case GE_OP: {
        int lval;
        int rval;
        if (term->left->left->type == IDENTIFIER)
        // PROBLEM HERE
            lval = find_ident_value(__interpret(term->left, env), peek(env->stack))->v.integer;
        else
            lval = __interpret(term->left, env)->v.integer;

        if (term->right->left->type == IDENTIFIER)
        // AND HERE
            rval = find_ident_value(__interpret(term->right, env), peek(env->stack))->v.integer;
        else
            rval = __interpret(term->right, env)->v.integer;

        VALUE *exit_term = calloc(1, sizeof(VALUE));
        exit_term->type = CONSTANT;

        switch (term->type) {
        case '+':
            exit_term->v.integer = lval + rval;
            break;
        case '-':
            exit_term->v.integer = lval - rval;
            break;
        case '*':
            exit_term->v.integer = lval * rval;
            break;
        case '/':
            exit_term->v.integer = lval / rval;
            break;
        case '%':
            exit_term->v.integer = lval % rval;
            break;
        case '>':
            exit_term->v.integer = lval > rval;
            break;
        case '<':
            exit_term->v.integer = lval < rval;
            break;
        case NE_OP:
            exit_term->v.integer = lval != rval;
            break;
        case EQ_OP:
            exit_term->v.integer = lval == rval;
            break;
        case LE_OP:
            exit_term->v.integer = lval <= rval;
            break;
        case GE_OP:
            exit_term->v.integer = lval >= rval;
            break;
        }
        return exit_term;
        break;
    }
    case IF:
        break;
    case WHILE:
        break;
    default:
        break;
    }
}

void print_bindings(FRAME *frame) {
    printf("*** BINDING LIST ***\n\n");
    BINDING *temp = frame->bindings;
    while (temp != NULL) {
        printf("%s\n", temp->name->lexeme);
        temp = temp->next;
    }
    printf("\n*** END OF BINDING LIST ***\n\n\n");
}

VALUE *find_ident_value(TOKEN *t, FRAME *frame) {
    BINDING *bindings = frame->bindings;
    while (bindings != NULL) {
        if (bindings->name == t) {
            VALUE *temp = bindings->val;
            return temp;
        }
        bindings = bindings->next;
    }
    // error(" unbound variable ");
}

VALUE *assing_value(TOKEN *t, FRAME *frame, VALUE *value) {
    if (frame != NULL) {
        BINDING *bindings = frame->bindings;
        while (bindings != NULL) {
            if (bindings->name == t) {
                bindings->val = value;
                return bindings->val;
            }
            bindings = bindings->next;
        }
    }
    // error(" unbound variable");
}

void declare(TOKEN *identifier, FRAME *frame) {
    BINDING *temp = frame->bindings;
    BINDING *new_bind = calloc(1, sizeof(BINDING));
    new_bind->name = identifier;

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

FRAME *extend_frame(FRAME *frame) {
    FRAME *newenv = calloc(1, sizeof(FRAME));
    BINDING *temp = frame->bindings;

    while (temp != NULL) {
        TOKEN *new_name = calloc(1, sizeof(TOKEN));
        char *temp_name = malloc(strlen(temp->name->lexeme) + 1);
        strcpy(temp_name, temp->name->lexeme);
        new_name->lexeme = temp_name;
        declare(new_name, newenv);

        VALUE *new_value = calloc(1, sizeof(VALUE));
        switch (temp->val->type) {
        case CONSTANT:
            new_value->type = CONSTANT;
            new_value->v.integer = temp->val->v.integer;
            assing_value(new_name, newenv, new_value);
            break;
        case STRING_LITERAL:
            new_value->type = STRING_LITERAL;
            strcpy(new_value->v.string, temp->val->v.string);
            assing_value(new_name, newenv, new_value);
            break;
        case FUNCTION:
            new_value->type = FUNCTION;
            CLOSURE *new_closure = calloc(1, sizeof(CLOSURE));
            new_closure->code = temp->val->v.closure->code;
            new_closure->env = newenv;
            assing_value(new_name, newenv, new_value);
            break;
        }
        temp = temp->next;
    }
    return newenv;
}