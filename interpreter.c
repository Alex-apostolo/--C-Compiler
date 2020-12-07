#include "interpreter.h"
#include "C.tab.h"
#include <stdio.h>

int last_frame_index = 0;

/*Function responsible for initializing*/
VALUE *interpret(NODE *term, ENV *env) {
    // The first frame in the environment is the global frame
    FRAME *global = env->frames;
    // Add global frame if it doesn't exist
    if (global == NULL) {
        global = calloc(1, sizeof(FRAME));
        global->index = last_frame_index;
        env->frames = global;
    }
    return interpret_(term,env);
}

VALUE *interpret_(NODE *term, ENV *env) {
    // initialize environment

    switch (term->type) {
    case LEAF:
        // Left child has value: STRING_LITERAL, IDENTIFIER, CONSTANT
        return interpret_(term->left, env);
        break;
    case IDENTIFIER:
        return term;
        break;
    case CONSTANT:
        // Term holds its value on the right child
        return term->right;
        break;
    case STRING_LITERAL:
        // Term holds its value on the left child
        return term->left;
    case APPLY:
        // Process calling of functions
        break;
    case VOID:
    case FUNCTION:
    case INT:
        break;
    case 'd':
        // Left child is an AST for Return type
        interpret_(term->left, env);
        // Right child is the Name and arguments
        interpret_(term->right, env);
        break;
    case 'D':
        // Left child is the Function definition
        interpret_(term->left, env);
        // Right child is the Function body
        if (term->right != NULL) {
            return interpret_(term->right, env);
        } else {
            return 0;
        }
        break;
    case 'F': {
        // Left child is the Name of function
        declare(interpret_(term->left, env), find_frame(last_frame_index, env));
        // Right child are the Arguments of function
        if (term->right != NULL) {
            interpret_(term->right, env);
        }
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
                return find_name(interpret_(term->left, env), env->frames);
            } else
                return interpret_(term->left, env);
        } else {
            // TODO: throw an error
            printf("Error no return type\n");
        }
        break;
    case '~':
        if (term->left->type == LEAF) { // If left is INT
            if (term->right->type == LEAF) {
                // Right child is the variable name
                declare(interpret_(term->right, env),
                        find_frame(last_frame_index, env));
            } else {
                // Right child is the AST "=" and we declare the variable before
                // assigning
                declare(interpret_(term->right->left, env),
                        find_frame(last_frame_index, env));
                interpret_(term->right, env);
            }
        } else if (term->left->type == '~') {
            // interpret_ left sequence
            interpret_(term->left, env);
            // Right will be a function or INT
            if (term->right->type == 'D')
                interpret_(term->right->left, env);
            else
                interpret_(term->right, env);
        } else if (term->left->type == 'D') {
            // Left will be a function
            interpret_(term->left->left, env);
            // Right will be a function or INT
            if (term->right->type == 'D')
                interpret_(term->right->left, env);
            else
                interpret_(term->right, env);
        }
        // print_bindings(find_frame(0,env));
        break;
    case ';':
        // Left child is the first item or a sequence; returned values are
        // ignored
        interpret_(term->left, env);
        // Right child is the last item and also the return value
        return interpret_(term->right, env);
        break;
    case '=':
        assign_value(interpret_(term->left, env), env->frames,
                     interpret_(term->right, env));
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
            lval = find_name(interpret_(term->left, env), env->frames);
        else
            lval = (int)interpret_(term->left, env);

        if (term->right->left->type == IDENTIFIER)
            rval = find_name(interpret_(term->right, env), env->frames);
        else
            rval = (int)interpret_(term->right, env);

        switch (term->type) {
        case '+':
            return lval + rval;
        case '-':
            return lval - rval;
        case '*':
            return lval * rval;
        case '/':
            return lval / rval;
        case '%':
            return lval % rval;
        case '>':
            return lval > rval;
        case '<':
            return lval < rval;
        case NE_OP:
            return lval != rval;
        case EQ_OP:
            return lval == rval;
        case LE_OP:
            return lval <= rval;
        case GE_OP:
            return lval >= rval;
        }
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
    BINDING *temp = frame->bindings;
    while (temp != NULL) {
        printf("%s\n", temp->name->lexeme);
        temp = temp->next;
    }
}

VALUE *find_name(TOKEN *t, FRAME *frame) {
    while (frame != NULL) {
        BINDING *bindings = frame->bindings;
        while (bindings != NULL) {
            if (bindings->name == t) {
                VALUE *temp = bindings->val;
                return temp;
            }
            bindings = bindings->next;
        }
        frame = frame->next;
    }
    // error(" unbound variable ");
}

FRAME *find_frame(int index, ENV *env) {
    FRAME *temp = env->frames;
    if (temp == NULL)
        fprintf(stderr,
                "Environment is empty, could not find specified frame\n");
    while (temp != NULL) {
        if (temp->index == index) {
            return temp;
        }
        temp = temp->next;
    }
    fprintf(stderr, "Could not find frame\n");
    return NULL;
}

VALUE *assign_value(TOKEN *t, FRAME *frame, VALUE *value) {
    while (frame != NULL) {
        BINDING *bindings = frame->bindings;
        while (bindings != NULL) {
            if (bindings->name == t) {
                bindings->val = value;
                return value;
            }
            bindings = bindings->next;
        }
        frame = frame->next;
    }
    // error(" unbound variable");
}

VALUE *declare(TOKEN *identifier, FRAME *frame) {
    BINDING *temp = frame->bindings;
    BINDING *new_bind = calloc(1, sizeof(BINDING));
    new_bind->name = identifier;
    new_bind->val = (VALUE *)0;
    if (new_bind != 0) {
        if (temp == NULL) {
            frame->bindings = new_bind;
            return new_bind->val;
        }
        while (temp->next != NULL)
            temp = temp->next;
        temp->next = new_bind;
        return new_bind->val;
    }
    // error (" binding allocation failed " );
}
