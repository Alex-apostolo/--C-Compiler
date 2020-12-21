#include "tac_constructors.c"
#include "tac_helper.c"

/*Function definitions*/
void _tac_generator(NODE *, TAC **, ENV_TAC *);
void *__tac_generator(NODE *, TAC **, ENV_TAC *);

void tac_generator(NODE *term, TAC **seq) {
    // Set up global environment
    _tac_generator(term, seq, env_tac_create_empty());

    // Find closures inside the generated TAC and run their body
    TAC *last = *seq;
    while (last->next != NULL) {
        if (last->op == CLOS_OP) {
            _tac_generator(last->args.clos->body, seq, env_tac_create_empty());
        }
        last = last->next;
    }
}

// Sets up global environment
void _tac_generator(NODE *term, TAC **seq, ENV_TAC *env) {
    switch (term->type) {
    case 'D':
        // Call the left to create a proc
        _tac_generator(term->left, seq, env);
        // Call the right for the body of the proc
        __tac_generator(term->right, seq, env);
        env_tac_clear(env);
        break;
    case 'd':
        // Right child is 'F'
        _tac_generator(term->right, seq, env);
        break;
    case 'F': {
        // Make PROC
        PROC *new_proc = proc_create(((TOKEN *)term->left->left)->lexeme, 0);
        // Find the number of arguments
        if (term->right == NULL)
            new_proc->arity = 0;
        else if (term->right->type == '~')
            new_proc->arity = 1;
        else {
            env->arguments = 0;
            _tac_generator(term->right, seq, env);
            new_proc->arity = env->arguments;
        }
        // Append to TAC
        append(seq, tac_create(PROC_OP, new_proc, NULL));
    } break;
    case ',':
        term->left->type == '~' ? env->arguments++ : _tac_generator(term->left, seq, env);
        term->right->type == '~' ? env->arguments++
                                 : _tac_generator(term->right, seq, env);
        break;
    case '~': {
        // Creates a Global TAC instruction
        // If the type of the left child is a leaf, then this indicates its a
        // declaration
        if (term->left->type == LEAF) {
            // Left child holds the type of the declaration
            // We ingore since we want to abstract for the mips

            // To understand the below assingmnets run the -f flag and go through the AST
            GLOBAL *new_global;
            if (term->right->type == LEAF)
                // Right child is the name of the declaration
                new_global =
                    global_create(((TOKEN *)term->right->left)->lexeme, term->left->left->type, NULL);
            else if (term->right->type == '=') {
                // Right child is the Node "="
                new_global = global_create(
                    ((TOKEN *)term->right->left->left)->lexeme, term->left->left->type, NULL);
                // switch on right Node to get the value
                switch (term->right->right->left->type) {
                case CONSTANT: {
                    VAL *new_val = val_create(CONSTANT, ((TOKEN *)term->right->right->left)->value);
                    new_global->val = new_val;
                    break;
                }
                case STRING_LITERAL: {
                    VAL *new_val = val_create(CONSTANT, ((TOKEN *)term->right->right->left)->lexeme);
                    new_global->val = new_val;
                    break;
                }
                    // case BOOLEAN:
                }
            }
            // // Prepends TAC instruction to sequence
            *seq = tac_create(GLOBAL_OP, new_global, *seq);
        } else {
            _tac_generator(term->left, seq, env);
            _tac_generator(term->right, seq, env);
        }
        break;
    }
    }
}

// Sets up the local environment
void *__tac_generator(NODE *term, TAC **seq, ENV_TAC *env) {
    switch (term->type) {
    case LEAF:
        // Left child has value: STRING_LITERAL, IDENTIFIER, CONSTANT
        return __tac_generator(term->left, seq, env);
        break;
    case IDENTIFIER:
    case CONSTANT:
    case STRING_LITERAL:
        // Interpret as TOKEN*
        return term;
        break;
    case APPLY: {
        if(term->right != NULL)
            __tac_generator(term->right, seq, env);
        // Creates a call TAC for the function: this will be a ja in MIPS
        env->latest_treg = "v0";
        CALL *new_call = call_create(((TOKEN *)term->left->left)->lexeme, 0, env->latest_treg, env->nvars, env->svars);
        append(seq, tac_create(CALL_OP, new_call, NULL));
        return env->latest_treg;
        break;
    }
    case VOID:
    case FUNCTION:
    case INT:
        break;
    case 'D': {
        // Make closure tac for function definition
        CLOS *new_clos = clos_create(term, ((TOKEN *)term->left->right->left->left)->lexeme);
        append(seq, tac_create(CLOS_OP, new_clos,NULL));
        break;
    }
    case CONTINUE:
    case BREAK:
        break;
    case RETURN: {
        // Left child is an AST of the expression whose value is to be returned
        // quite similar to the interpreter
        if (term->left != NULL) {
            RET *new_ret;
            if (term->left->type == LEAF) {
                TOKEN *temp = __tac_generator(term->left, seq, env);
                if (term->left->left->type == IDENTIFIER) 
                    new_ret = ret_create(IDENTIFIER, temp->lexeme);
                if (term->left->left->type == CONSTANT) 
                    new_ret = ret_create(CONSTANT, temp->value);
            } else {
                new_ret = ret_create(TREG, __tac_generator(term->left, seq, env));
            }
            append(seq, tac_create(RET_OP, new_ret, NULL));
        } else {
            // TODO: throw an error
            printf("Error no return type\n");
        }
        break;
    }
    case '~':
        // Left child is the type
        __tac_generator(term->left, seq, env);
        // Right child is the variable name
        TOKEN *identifier = __tac_generator(term->right, seq, env);
        appendVAR(env->svars, var_create(identifier->lexeme, IDENTIFIER, NULL),env);
        break;
    case ';':
        // Left child is the first item or a sequence; returned values are
        // ignored
        __tac_generator(term->left, seq, env);
        // Right child is the last item and also the return value
        return __tac_generator(term->right, seq, env);
        break;
    case ',':
        __tac_generator(term->left, seq, env);
        __tac_generator(term->right, seq, env);
        break;
    case '=': {
        // Creates TAC for rhs of expression
        if (term->right->type == LEAF) {

            TAC *rhs = create_load_TAC(__tac_generator(term->right, seq, env),env);
            append(seq, rhs);
        } else
            __tac_generator(term->right, seq, env);
        // Creates TAC for lhs of expression
        TAC *lhs = create_store_TAC(__tac_generator(term->left, seq, env),env,seq);
        append(seq, lhs);
        TOKEN *new_tok = calloc(1,sizeof(TOKEN));
        new_tok->type = IDENTIFIER;
        new_tok->lexeme = lhs->args.store->value;
        return new_tok;
        break;
    }
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

        // Creates load TAC instructions
        char *src1;
        if (term->left->type != LEAF) {
            src1 = __tac_generator(term->left, seq, env);
        } else {
            TAC *temp = create_load_TAC(__tac_generator(term->left, seq, env),env);
            append(seq, temp);
            src1 = temp->args.load->treg;
        }

        // Creates load TAC instructions
        char *src2;
        if (term->right->type != LEAF) {
            src2 = __tac_generator(term->right, seq, env);
        } else {
            TAC *temp = create_load_TAC(__tac_generator(term->right, seq, env),env);
            append(seq, temp);
            src2 = temp->args.load->treg;
        }

        char *dst = treg_generator(env);
        EXPR *new_expr = expr_create(src1, src2, dst);
        append(seq, tac_create(term->type, new_expr, NULL));
        return dst;
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