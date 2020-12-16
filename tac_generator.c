#include "tac_constructors.c"

/*Function definitions*/
char *treg_generator();
char *label_generator();
TAC *create_load_TAC(TOKEN *term);
TAC *create_store_TAC(TOKEN *term);
void appendVAR(VAR **, VAR *);
void append(TAC **, TAC *);
void _tac_generator(NODE *, TAC **);
void *__tac_generator(NODE *, TAC **);

/*Global Variables*/
int ntreg = 0;
char *latest_treg;
int nlabel = 0;
char *latest_label;
int arguments = 0;
int *nvars;
VAR **svars;

void tac_generator(NODE *term, TAC **seq) {
    svars = calloc(1, sizeof(VAR *));
    nvars = calloc(1, sizeof(int));
    // Set up global environment
    _tac_generator(term, seq);

    // Find closures inside the generated TAC and run their body
    TAC *last = *seq;
    while (last->next != NULL) {
        if (last->op == CLOS_OP) {
            ntreg = 0;
            arguments = 0;
            svars = calloc(1, sizeof(VAR *));
            nvars = calloc(1, sizeof(int));
            _tac_generator(last->args.clos->body, seq);
        }
        last = last->next;
    }
}

// Sets up global environment
void _tac_generator(NODE *term, TAC **seq) {
    switch (term->type) {
    case 'D':
        // Call the left to create a proc
        _tac_generator(term->left, seq);
        // Call the right for the body of the proc
        __tac_generator(term->right, seq);
        ntreg = 0;
        arguments = 0;
        svars = calloc(1, sizeof(VAR *));
        nvars = calloc(1, sizeof(int));
        break;
    case 'd':
        // Right child is 'F'
        _tac_generator(term->right, seq);
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
            arguments = 0;
            _tac_generator(term->right, seq);
            new_proc->arity = arguments;
        }
        // Append to TAC
        append(seq, tac_create(PROC_OP, new_proc, NULL));

        // Make BLOCK
        BLOCK *new_block = block_create(nvars, svars);
        // Append to TAC
        append(seq, tac_create(BLOCK_OP, new_block, NULL));
    } break;
    case ',':
        term->left->type == '~' ? arguments++ : _tac_generator(term->left, seq);
        term->right->type == '~' ? arguments++
                                 : _tac_generator(term->right, seq);
        break;
    case '~': {
        // Creates a Global TAC instruction
        // If the type of the left child is a leaf, then this indicates its a
        // declaration
        if (term->left->type == LEAF) {
            // Left child holds the type of the declaration
            // We ingore since we want to abstract for the mips

            GLOBAL *new_global;
            if (term->right->type == LEAF)
                // Right child is the name of the declaration
                new_global =
                    global_create(((TOKEN *)term->right->left)->lexeme, NULL);
            else if (term->right->type == '=') {
                // Right child is the Node "="
                new_global = global_create(
                    ((TOKEN *)term->right->left->left)->lexeme, NULL);
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
            _tac_generator(term->left, seq);
            _tac_generator(term->right, seq);
        }
        break;
    }
    }
}

// Sets up the local environment
void *__tac_generator(NODE *term, TAC **seq) {
    switch (term->type) {
    case LEAF:
        // Left child has value: STRING_LITERAL, IDENTIFIER, CONSTANT
        return __tac_generator(term->left, seq);
        break;
    case IDENTIFIER:
    case CONSTANT:
    case STRING_LITERAL:
        // Interpret as TOKEN*
        return term;
        break;
    case APPLY: {
        // Creates a call TAC for the function: this will be a ja in MIPS
        CALL *new_call = call_create(((TOKEN *)term->left->left)->lexeme, 0, treg_generator());
        append(seq, tac_create(CALL_OP, new_call, NULL));
        return latest_treg;
        break;
    }
    case VOID:
    case FUNCTION:
    case INT:
        break;
    case 'd':
        // Left child is an AST for Return type
        // __tac_generator(term->left, seq);
        // // Right child is the Name and arguments
        // __tac_generator(term->right, seq);
        break;
    case 'D': {
        // Make closure tac for function definition
        CLOS *new_clos = clos_create(term, ((TOKEN *)term->left->right->left->left)->lexeme);
        append(seq, tac_create(CLOS_OP, new_clos,NULL));
        // Appends to svars, similar funciton to bindings in interpreter
        appendVAR(svars, var_create(new_clos->name, NULL));
        break;
    }
    case CONTINUE:
    case BREAK:
        break;
    case RETURN: {
        // Left child is an AST of the expression whose value is to be returned
        // TODO: handle the case where left child is identifier and the case for
        // everything else
        if (term->left != NULL) {
            RET *new_ret;
            if (term->left->type == LEAF) {
                TOKEN *temp = __tac_generator(term->left, seq);
                if (term->left->left->type == IDENTIFIER) 
                    new_ret = ret_create(IDENTIFIER, temp->lexeme);
                if (term->left->left->type == CONSTANT) 
                    new_ret = ret_create(CONSTANT, temp->value);
            } else {
                new_ret = ret_create(TREG, __tac_generator(term->left, seq));
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
        __tac_generator(term->left, seq);
        // Right child is the variable name
        TOKEN *identifier = __tac_generator(term->right, seq);
        appendVAR(svars, var_create(identifier->lexeme, NULL));
        break;
    case ';':
        // Left child is the first item or a sequence; returned values are
        // ignored
        __tac_generator(term->left, seq);
        // Right child is the last item and also the return value
        return __tac_generator(term->right, seq);
        break;
    case '=': {
        // Creates TAC for rhs of expression
        if (term->right->type == LEAF) {

            TAC *rhs = create_load_TAC(__tac_generator(term->right, seq));
            append(seq, rhs);
        } else
            __tac_generator(term->right, seq);
        // Creates TAC for lhs of expression
        TAC *lhs = create_store_TAC(__tac_generator(term->left, seq));
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
            src1 = __tac_generator(term->left, seq);
        } else {
            TAC *temp = create_load_TAC(__tac_generator(term->left, seq));
            append(seq, temp);
            src1 = temp->args.load->treg;
        }

        // Creates load TAC instructions
        char *src2;
        if (term->right->type != LEAF) {
            src2 = __tac_generator(term->right, seq);
        } else {
            TAC *temp = create_load_TAC(__tac_generator(term->right, seq));
            append(seq, temp);
            src2 = temp->args.load->treg;
        }

        char *dst = treg_generator();
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

char *my_itoa(int num) {
    char *str = malloc(100 * sizeof(char));
    sprintf(str, "%d", num);
    return str;
}

char *treg_generator() {
    // Maximum of 3 characters, since we have 32 registers at our disposal
    char *str = malloc(3 * sizeof(char));
    snprintf(str, sizeof(str), "t%d", ntreg++);
    latest_treg = str;
    // Append register to svars
    appendVAR(svars, var_create(str, NULL));
    return str;
}

char *label_generator() {
    // Maximum of 5 characters, more than enough for the labels of our programs
    char *str = malloc(5 * sizeof(char));
    snprintf(str, sizeof(str), "L%d", nlabel++);
    latest_label = str;
    return str;
}

void appendVAR(VAR **seq, VAR *new_node) {
    (*nvars)++;
    if (seq == NULL) {
        return;
    }
    new_node->next = NULL;

    if (*seq == NULL) {
        *seq = new_node;
        return;
    }

    VAR *last = *seq;
    while (last->next != NULL) {
        last = last->next;
    }
    last->next = new_node;
    return;
}

void append(TAC **seq, TAC *new_node) {
    if (seq == NULL) {
        return;
    }
    new_node->next = NULL;

    if (*seq == NULL) {
        *seq = new_node;
        return;
    }

    TAC *last = *seq;
    while (last->next != NULL) {
        last = last->next;
    }
    last->next = new_node;
    return;
}

TAC *create_load_TAC(TOKEN *term) {
    char *treg = treg_generator();
    LOAD *new_load;
    if (term->type == IDENTIFIER) 
        new_load = load_create(treg, IDENTIFIER, term->lexeme);
    else {
        char *temp = malloc(100 * sizeof(char));
        snprintf(temp, 100, "%d", term->value);
        new_load = load_create(treg, CONSTANT, temp);
    }
    return tac_create(LOAD_OP, new_load, NULL);
}

TAC *create_store_TAC(TOKEN *term) {
    STORE *new_store = store_create(latest_treg, NULL);
    if (term->type == IDENTIFIER) {
        new_store = store_create(latest_treg, term->lexeme);
    } else {
        char *temp = malloc(100 * sizeof(char));
        snprintf(temp, 100, "%d", term->value);
        new_store = store_create(latest_treg, temp);
    }
    return tac_create(STORE_OP, new_store, NULL);
}

// Refactor for basic blocks
void printTAC(FILE *file, BB *bb) {
    BB *bbtemp = bb;
    while (bbtemp != NULL) {
        TAC *temp = *(bbtemp->leader);
        while (temp != NULL) {
            switch (temp->op) {
            case PROC_OP:
                fprintf(file, "\nproc %s %d\n", temp->args.proc->name,
                        temp->args.proc->arity);
                break;
            case CALL_OP:
                fprintf(file, "%s = call %s\n", temp->args.call->store,
                        temp->args.call->name);
                break;
            case BLOCK_OP:
                fprintf(file, "block %d\n", *(temp->args.block->nvars));
                break;
            case GLOBAL_OP:
                temp->args.glob->val == NULL
                    ? fprintf(file, "global %s\n", temp->args.glob->name)
                    : fprintf(file, "global %s %d\n", temp->args.glob->name,
                              temp->args.glob->val->v.integer);
                break;

            case LOAD_OP:
                if (temp->args.load->type == IDENTIFIER)
                    fprintf(file, "load %s %s\n", temp->args.load->treg,
                            temp->args.load->val.identifier);
                else
                    fprintf(file, "load %s %s\n", temp->args.load->treg,
                            temp->args.load->val.constant);
                break;
            case STORE_OP:
                fprintf(file, "store %s %s\n", temp->args.store->treg,
                        temp->args.store->value);
                break;
            case CLOS_OP:
                fprintf(file, "closure %s\n", temp->args.clos->name);
                break;
            case RET_OP:
                // if returnee is main then li $v blabla otherwise jr
                switch (temp->args.ret->type) {
                case IDENTIFIER:
                    fprintf(file, "return %s\n",
                            temp->args.ret->val.identifier);
                    break;
                case CONSTANT:
                    fprintf(file, "return %d\n", temp->args.ret->val.constant);
                    break;
                case TREG:
                    fprintf(file, "return %s\n", temp->args.ret->val.treg);
                    break;
                }
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
            case GE_OP:
                switch (temp->op) {
                case '+':
                    fprintf(file, "add");
                    break;
                case '-':
                    fprintf(file, "sub");
                    break;
                case '*':
                    fprintf(file, "mul");
                    break;
                case '/':
                    fprintf(file, "div");
                    break;
                case '%':
                    fprintf(file, "mod");
                    break;
                case '>':
                    fprintf(file, "gret");
                    break;
                case '<':
                    fprintf(file, "less");
                    break;
                case NE_OP:
                    fprintf(file, "ne");
                    break;
                case EQ_OP:
                    fprintf(file, "eq");
                    break;
                case LE_OP:
                    fprintf(file, "eqle");
                    break;
                case GE_OP:
                    fprintf(file, "eqge");
                    break;
                }
                fprintf(file, " %s %s %s\n", temp->args.expr->src1,
                        temp->args.expr->src2, temp->args.expr->dst);
                break;
            default:
                break;
            }
            temp = temp->next;
        }
        bbtemp = bbtemp->next;
    }
}