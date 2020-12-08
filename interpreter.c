#include "interpreter.h"
#include "C.tab.h"
#include <stdio.h>

int last_frame_index;

/*Function responsible for initializing*/
VALUE *interpret(NODE *term, BB *env) {
    // The first frame in the environment is the global frame
    FRAME *global = env->frame_leader;
    // Add global frame if it doesn't exist
    if (global == NULL) {
        global = calloc(1, sizeof(FRAME));
        global->index = last_frame_index;
        env->frame_leader = global;
    }
    return interpret_(term, env);
}

VALUE *interpret_(NODE *term, BB *env) {
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
        return interpret_(term->right, env);
        break;
    case 'D': {
        // Creates new closure and assigns it to function defined
        VALUE *new_value = calloc(1,sizeof(VALUE));
        CLOSURE *new_closure = calloc(1, sizeof(CLOSURE));
        new_closure->code = term->right;
        new_closure->env = env->frame_leader;
        new_value->v.closure = new_closure;
        assign_value(interpret_(term->left, env), env->frame_leader, new_value);

        // after calling the function make the environment concrete
        break;
    }
    case 'F': {
        // Right child are the Arguments of function
        if (term->right != NULL) {
            interpret_(term->right, env);
        }
        // Left child is the Name of function
        TOKEN *t = interpret_(term->left, env);
        declare(t, find_frame(last_frame_index, env));
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
                return find_ident_value(interpret_(term->left, env),
                                 env->frame_leader);
            } else
                return interpret_(term->left, env);
        } else {
            // TODO: throw an error
            printf("Error no return type\n");
        }
        break;
    case '~':
        if (term->left->type == LEAF) {
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
        } else {
            interpret_(term->left, env);
            interpret_(term->right, env);
        }
        break;
    case ';':
        // Left child is the first item or a sequence; returned values are
        // ignored
        interpret_(term->left, env);
        // Right child is the last item and also the return value
        return interpret_(term->right, env);
        break;
    case '=':
        assign_value(interpret_(term->left, env), env->frame_leader,
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
            lval = find_ident_value(interpret_(term->left, env), env->frame_leader);
        else
            lval = (int)interpret_(term->left, env);

        if (term->right->left->type == IDENTIFIER)
            rval = find_ident_value(interpret_(term->right, env), env->frame_leader);
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
    printf("\n\n\n*** BINDING LIST ***\n\n");
    BINDING *temp = frame->bindings;
    while (temp != NULL) {
        printf("%s\n", temp->name->lexeme);
        temp = temp->next;
    }
    printf("\n*** END OF BINDING LIST ***");
}

VALUE *find_ident_value(TOKEN *t, FRAME *frame) {
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

FRAME *find_frame(int index, BB *env) {
    FRAME *temp = env->frame_leader;
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

// FRAME *extend_frame(FRAME *env, NODE *ids, NODE *args) {
//     FRAME *newenv = make_frame(NULL, NULL); // note env = NULL
//     BINDING *bindings = NULL;
//     for (NODE *ip = ids,NODE *ap = args; (ip != NULL) && (ap != NULL); ip ->
//     right, ap->right) {
//         bindings = make_binding(ip -> left, interpret(ap->left,
//         env),bindings)
//     }
//     newenv->bindings = bindings;
//     return newenv;
// }