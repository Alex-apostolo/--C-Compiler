#include "tac_generator.h"
#include "C.tab.h"
#include "nodes.h"
#include <stdio.h>
#include <stdlib.h>

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
        if (last->op == CLOS_OP){
            svars = calloc(1, sizeof(VAR *));
            nvars = calloc(1, sizeof(int));
            _tac_generator(last->args.clos.body, seq);
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
        break;
    case 'd':
        // Right child is 'F'
        _tac_generator(term->right, seq);
        break;
    case 'F': {
        // Creates a Process TAC instruction
        PROC *new_proc = calloc(1, sizeof(PROC));
        new_proc->name = ((TOKEN *)term->left->left)->lexeme;
        if (term->right == NULL)
            new_proc->arity = 0;
        else if (term->right->type == '~')
            new_proc->arity = 1;
        else {
            arguments = 0;
            _tac_generator(term->right, seq);
            new_proc->arity = arguments;
        }
        TAC *proc = calloc(1, sizeof(TAC));
        proc->args.proc = *new_proc;
        proc->op = PROC_OP;
        proc->next = NULL;

        // Append to TAC
        append(seq, proc);

        // Make BLOCK
        BLOCK *new_block = calloc(1, sizeof(BLOCK));
        new_block->nvars = nvars;
        new_block->svars = svars;
        
        TAC *block = calloc(1, sizeof(TAC));
        block->op = BLOCK_OP;
        block->next = NULL;
        block->args.block = *new_block;
        append(seq, block);
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

            // DONT FORGET TO REFACTOR
            GLOBAL *global = calloc(1, sizeof(GLOBAL));
            if (term->right->type == LEAF) {
                // Right child is the name of the declaration
                global->name = ((TOKEN *)term->right->left)->lexeme;
                global->val = NULL;
            } else if (term->right->type == '=') {
                // Right child is the Node "="
                global->name = ((TOKEN *)term->right->left->left)->lexeme;
                // switch on right Node to get the value
                switch (term->right->right->left->type) {
                case CONSTANT: {
                    VAL *val = calloc(1, sizeof(VAL));
                    global->val = val;
                    global->val->type = CONSTANT;
                    global->val->v.integer =
                        ((TOKEN *)term->right->right->left)->value;
                    break;
                }
                case STRING_LITERAL: {
                    VAL *val = calloc(1, sizeof(VAL));
                    global->val = val;
                    global->val->type = STRING_LITERAL;
                    global->val->v.string =
                        ((TOKEN *)term->right->right->left)->lexeme;
                    break;
                }

                    // case BOOLEAN:
                }
            }
            // Prepends TAC instruction to sequence
            TAC *temp = calloc(1, sizeof(TAC));
            temp->op = GLOBAL_OP;
            temp->args.glob = *global;
            temp->next = *seq;
            *seq = temp;

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
        TAC *call = calloc(1, sizeof(TAC));
        call->op = CALL_OP;
        call->args.call.name = ((TOKEN *)term->left->left)->lexeme;
        call->args.call.store = treg_generator();
        call->next = NULL;
        append(seq, call);
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
        CLOS *new_clos = calloc(1, sizeof(CLOS));
        new_clos->name = ((TOKEN *)term->left->right->left->left)->lexeme;
        new_clos->body = term;
        TAC *clos = calloc(1, sizeof(TAC));
        clos->args.clos = *new_clos;
        clos->op = CLOS_OP;
        append(seq, clos);

        VAR *new_var = calloc(1, sizeof(VAR));
        new_var->name = new_clos->name;
        new_var->next = NULL;
        appendVAR(svars, new_var);
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
            TAC *ret = (TAC *)malloc(sizeof(TAC));
            ret->next = NULL;
            ret->op = RETURN;
            if (term->left->type == LEAF) {
                TOKEN *temp = __tac_generator(term->left, seq);
                if (term->left->left->type == IDENTIFIER) {
                    ret->args.ret.type = IDENTIFIER;
                    ret->args.ret.val.identifier = temp->lexeme;
                } else if (term->left->left->type == CONSTANT) {
                    ret->args.ret.type = CONSTANT;
                    ret->args.ret.val.constant = temp->value;
                }
            } else {
                ret->args.ret.type = TREG;
                ret->args.ret.val.treg = __tac_generator(term->left, seq);
            }
            append(seq, ret);
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
        __tac_generator(term->right, seq);
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

        // Creates add TAC instruction
        TAC *add = (TAC *)malloc(sizeof(TAC));
        add->next = NULL;
        add->op = term->type;
        // Check if left or right childs are APPLY

        // Creates load TAC instructions
        TAC *src1;
        if (term->left->type != LEAF) {
            src1 = __tac_generator(term->left, seq);
            add->args.expr.src1 = (char *)src1;
        } else {
            src1 = create_load_TAC(__tac_generator(term->left, seq));
            append(seq, src1);
            add->args.expr.src1 = src1->args.load.treg;
        }

        // Creates load TAC instructions
        TAC *src2;
        if (term->right->type != LEAF) {
            src2 = __tac_generator(term->right, seq);
            add->args.expr.src2 = (char *)src2;
        } else {
            src2 = create_load_TAC(__tac_generator(term->right, seq));
            append(seq, src2);
            add->args.expr.src2 = src2->args.load.treg;
        }

        add->args.expr.dst = treg_generator();
        // prepend
        append(seq, add);
        return add->args.expr.dst;
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

    VAR *new_var = calloc(1, sizeof(VAR));
    new_var->name = str;
    new_var->next = NULL;
    appendVAR(svars, new_var);
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
    TAC *load = malloc(sizeof(TAC));
    load->next = NULL;
    load->op = LOAD_OP;
    load->args.load.treg = treg;
    if (term->type == IDENTIFIER) {
        load->args.load.type = IDENTIFIER;
        load->args.load.val.identifier = term->lexeme;
    } else {
        char *temp = malloc(100 * sizeof(char));
        snprintf(temp, 100, "%d", term->value);
        load->args.load.type = CONSTANT;
        load->args.load.val.constant = temp;
    }
    return load;
}

TAC *create_store_TAC(TOKEN *term) {
    char *treg = latest_treg;
    TAC *store = malloc(sizeof(TAC));
    store->next = NULL;
    store->op = STORE_OP;
    store->args.store.treg = treg;
    if (term->type == IDENTIFIER) {
        store->args.store.value = term->lexeme;
        VAR *new_var = calloc(1, sizeof(VAR));
        new_var->name = term->lexeme;
        new_var->next = NULL;
        appendVAR(svars, new_var);
    } else {
        char *temp = malloc(100 * sizeof(char));
        snprintf(temp, 100, "%d", term->value);
        store->args.store.value = temp;
    }
    return store;
}

// Refactor for basic blocks
void printTAC(FILE *file, BB *bb) {
    BB *bbtemp = bb;
    while (bbtemp != NULL) {
        TAC *temp = *(bbtemp->leader);
        while (temp != NULL) {
            switch (temp->op) {
            case PROC_OP:
                fprintf(file, "\nproc %s %d\n", temp->args.proc.name,
                        temp->args.proc.arity);
                break;
            case CALL_OP:
                fprintf(file, "%s = call %s\n", temp->args.call.store,
                        temp->args.call.name);
                break;
            case BLOCK_OP:
                fprintf(file, "block %d\n", *(temp->args.block.nvars));
                break;
            case GLOBAL_OP:
                temp->args.glob.val == NULL
                    ? fprintf(file, "global %s\n", temp->args.glob.name)
                    : fprintf(file, "global %s %d\n", temp->args.glob.name,
                              temp->args.glob.val->v.integer);
                break;

            case LOAD_OP:
                if (temp->args.load.type == IDENTIFIER)
                    fprintf(file, "load %s %s\n", temp->args.load.treg,
                            temp->args.load.val.identifier);
                else
                    fprintf(file, "load %s %s\n", temp->args.load.treg,
                            temp->args.load.val.constant);
                break;
            case STORE_OP:
                fprintf(file, "store %s %s\n", temp->args.store.treg,
                        temp->args.store.value);
                break;
            case CLOS_OP:
                fprintf(file, "closure %s\n", temp->args.clos.name);
                break;
            case RETURN:
                // if returnee is main then li $v blabla otherwise jr
                switch (temp->args.ret.type) {
                case IDENTIFIER:
                    fprintf(file, "return %s\n", temp->args.ret.val.identifier);
                    break;
                case CONSTANT:
                    fprintf(file, "return %d\n", temp->args.ret.val.constant);
                    break;
                case TREG:
                    fprintf(file, "return %s\n", temp->args.ret.val.treg);
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
                fprintf(file, " %s %s %s\n", temp->args.expr.src1,
                        temp->args.expr.src2, temp->args.expr.dst);
                break;
            default:
                break;
            }
            temp = temp->next;
        }
        bbtemp = bbtemp->next;
    }
}