#include "interpreter_constructors.c"

/*FUNCTION DEFINITIONS: these functions are declared locally and not in the .h
 * file because we want to hide them from the outside scope*/
VALUE *_interpreter(NODE *, ENV *);
VALUE *execute(char *, ENV *, int, NODE *);


// Initialization of the interpreter
VALUE *interpret(NODE *term, ENV *env) {
    // Add global frame if it doesn't exist
    if (env->global == NULL)
        env->global = frame_create(NULL);
    // Create a new Stack which will be used for storing frames
    env->stack = createStack(1024 * 1024);
    // Push global frame to stack
    push(env->stack, env->global);

    // Initiates global frame and saves it to stack
    _interpreter(term, env);

    // Pop global frame from Stack
    env->global = pop(env->stack);

    // Execute "main"
    return execute("main", env, 0, NULL);
}

// Called after the initialization of the interpreter is completed
VALUE *_interpreter(NODE *term, ENV *env) {
    switch (term->type) {
    case LEAF:
        // Left child has value: STRING_LITERAL, IDENTIFIER, CONSTANT
        return _interpreter(term->left, env);
        break;
    case IDENTIFIER:
        if (strcmp(((TOKEN *)term)->lexeme, "true") == 0)
            return value_create(BOOL_OP, (void *)1);
        if (strcmp(((TOKEN *)term)->lexeme, "false") == 0)
            return value_create(BOOL_OP, (void *)0);

        return (VALUE *)term;
        break;
    case CONSTANT: {
        // Term holds its value on the right child
        return value_create(CONSTANT, (void *)term->right);
        break;
    }
    case STRING_LITERAL: {
        // Term holds its value on the left child
        return value_create(STRING_LITERAL, (void *)term->left);
    }
    case APPLY: {
        // Call function
        char *name = ((TOKEN *)_interpreter(term->left, env))->lexeme;
        if (strcmp(name, "print") == 0) {
            VALUE *res = _interpreter(term->right, env);
            printf("%s", res->v.string);
            return res;
        }
        VALUE *result = execute(name, env, 1, term->right);
        if (result->v.integer == -2) {
            result = execute(name, env, 0, term->right);
            if (result->v.integer == -2) {
                printf("Error could not find function\n\n");
                return NULL;
            }
        }
        // Pop from frame from stack
        pop(env->stack);
        // Return result
        return result;
        break;
    }
    case VOID:
    case FUNCTION:
    case INT:
        break;
    case 'd':
        // Left child is an AST for Return type
        _interpreter(term->left, env);
        // Right child is the Name and arguments
        return _interpreter(term->right, env);
        break;
    case 'D': {
        // TODO: refactor term->left->right->right
        // Creates new closure
        CLOSURE *new_closure = closure_create(term->left->right->right,
                                              term->right, peek(env->stack));
        // Assigns closure to a new VALUE struct
        VALUE *new_value = value_create(FUNCTION, (void *)new_closure);
        // Name of Function
        TOKEN *name = (TOKEN *)_interpreter(term->left, env);
        // Assings closure to the bindings of the environment
        assing_value(name, peek(env->stack), new_value, env);
        break;
    }
    case 'F': {
        // Right child are the Arguments of function
        // if (term->right != NULL) {
        //     _interpreter(term->right, env);
        // }
        // Left child is the Name of function
        TOKEN *t = (TOKEN *)_interpreter(term->left, env);
        declare(t, peek(env->stack));
        return (VALUE *)t;
        break;
    }
    case ',':
        _interpreter(term->right, env);
        _interpreter(term->left, env);
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
                return find_ident_value((TOKEN *)_interpreter(term->left, env),
                                        peek(env->stack), env);
            } else
                return _interpreter(term->left, env);
        } else {
            // TODO: throw an error
            printf("Error no return type\n");
        }
        break;
    case '~':
        if (term->left->type == LEAF) {
            if (term->right->type == LEAF) {
                // Right child is the variable name
                declare((TOKEN *)_interpreter(term->right, env),
                        peek(env->stack));
            } else {
                // Right child is the AST "=" and we declare the variable before
                // assigning
                declare((TOKEN *)_interpreter(term->right->left, env),
                        peek(env->stack));
                _interpreter(term->right, env);
            }
        } else {
            _interpreter(term->left, env);
            _interpreter(term->right, env);
        }
        break;
    case ';':
        // Left child is the first item or a sequence; returned values are
        // ignored
        _interpreter(term->left, env);
        // Right child is the last item and also the return value
        return _interpreter(term->right, env);
        break;
    case '=':
        assing_value((TOKEN *)_interpreter(term->left, env), peek(env->stack),
                     _interpreter(term->right, env), env);
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
            lval = find_ident_value((TOKEN *)_interpreter(term->left, env),
                                    peek(env->stack), env)
                       ->v.integer;
        else
            lval = _interpreter(term->left, env)->v.integer;

        if (term->right->left->type == IDENTIFIER)
            rval = find_ident_value((TOKEN *)_interpreter(term->right, env),
                                    peek(env->stack), env)
                       ->v.integer;
        else
            rval = _interpreter(term->right, env)->v.integer;

        VALUE *exit_term = value_create(CONSTANT, NULL);

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
            exit_term->v.boolean = lval > rval;
            break;
        case '<':
            exit_term->v.boolean = lval < rval;
            break;
        case NE_OP:
            exit_term->v.boolean = lval != rval;
            break;
        case EQ_OP:
            exit_term->v.boolean = lval == rval;
            break;
        case LE_OP:
            exit_term->v.boolean = lval <= rval;
            break;
        case GE_OP:
            exit_term->v.boolean = lval >= rval;
            break;
        }
        return exit_term;
        break;
    }
    case IF: {
        // Making symbols for true and else didn't work
        NODE *antecedent = term->left;
        NODE *consequent =
            term->right->type == ELSE ? term->right->left : term->right;
        NODE *alternate = term->right->type == ELSE ? term->right->right : NULL;

        VALUE *result = _interpreter(antecedent, env);
        if (result->v.boolean != 1) {
            // execute alternate
            if (alternate == NULL)
                break;
            _interpreter(alternate, env);
        } else {
            // execute consequent
            _interpreter(consequent, env);
        }

        break;
    }
    case WHILE:
        break;
    default:
        break;
    }
    return NULL;
}

// Function responsible for finding or declaring a frame in an environment and
// extending its environment

// First argument is the frame we are searching or declaring

// Second argument is the environment we want to extend

// Third argument is a boolean(declared as integer for portability) indicating
// whether we called the function from a local or global state

// Fourth argument are the arguments of the frame provided
VALUE *execute(char *frame, ENV *env, int local, NODE *args) {
    // Search for frame and execute it
    int func_exists = 0;
    BINDING *temp = local ? peek(env->stack)->bindings : env->global->bindings;
    while (temp != NULL) {
        if (strcmp(temp->name->lexeme, frame) == 0) {
            func_exists = 1;

            FRAME *extension =
                local ? extend_frame(peek(env->stack)) : frame_create(NULL);
            push(env->stack, extension);
            if (temp->val->v.closure->code == NULL) {
                fprintf(stderr, "Error function body cannot be empty\n\n");
                return value_create(CONSTANT, (void *)-1);
            }
            // Run argument part of function
            if (temp->val->v.closure->params != NULL) {
                _interpreter(temp->val->v.closure->params, env);
                // run apply part of arguments
                if (args != NULL)
                    _interpreter(args, env);
            }
            // Run main part of function
            return _interpreter(temp->val->v.closure->code, env);
        }
        temp = temp->next;
    }

    // If the function doesn't exist return -2
    if (func_exists == 0) {
        // fprintf(stderr, "Error '%s' function could not be located\n\n",
        // frame);
        return value_create(CONSTANT, (void *)-2);
    }
    return NULL;
}