#include "tac_generator.h"
#include "C.tab.h"
#include "nodes.h"
#include <stdio.h>
#include <stdlib.h>

/*Function definitions*/
void appendbbb(BB *, TAC *);
void append(TAC **seq, TAC *new_node);
BB *appendBB(BB **, BB *);
char *treg_generator();
char *label_generator();
TAC *create_load_TAC(TOKEN *term);
TAC *create_store_TAC(TOKEN *term);
void _tac_generator(NODE *, TAC **);

/*Global Variables*/
int ntreg = 0;
char *latest_treg;
int nlabel = 0;
char *latest_label;
int nvars = 0;
char *svars[31];

int arguments = 0;

BB *tac_generator(NODE *term) {
    TAC **seq = calloc(1, sizeof(TAC *));
    _tac_generator(term, seq);
}

// return values for constant literals etc
void _tac_generator(NODE *term, TAC **seq) {
    switch (term->type) {
    case 'D':
        // Call the left to create a proc
        _tac_generator(term->left, seq);
        // Call the right for the body of the proc
        // tac_generator(term->right,seq);
        break;
    case 'F': {
        // Creates a Process TAC instruction
        PROC *new_proc = calloc(1, sizeof(PROC));
        new_proc->name = ((TOKEN *)term->left)->lexeme;
        if (term->right == NULL)
            new_proc->arity = 0;
        else if (term->right == '~')
            new_proc->arity = 1;
        else {
            arguments = 0;
            _tac_generator(term->right, seq);
            new_proc->arity = arguments;
        }
        // Append to TAC
        append(seq, new_proc);
    } break;
    case ',':
        term->left->type == '~' ? arguments++ : _tac_generator(term->left, seq);
        term->right->type == '~' ? arguments++
                                 : _tac_generator(term->right, seq);
    case '~': {
        // Creates a Global TAC instruction
        // If the type of the left child is a leaf, then this indicates its a
        // declaration
        if (term->left->type == LEAF && term->right->type != 'F') {
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
    return str;
}

char *label_generator() {
    // Maximum of 5 characters, more than enough for the labels of our programs
    char *str = malloc(5 * sizeof(char));
    snprintf(str, sizeof(str), "L%d", nlabel++);
    latest_label = str;
    return str;
}

void append(TAC **seq, TAC *new_node) {
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

void appendbbb(BB *bb, TAC *new_node) {
    new_node->next = NULL;

    if (bb->leader == NULL) {
        // alocate space for bb->leader then assign
        bb->leader = calloc(1, sizeof(TAC *));
        *(bb->leader) = new_node;
        return;
    }
    if (*(bb->leader) == NULL) {
        *(bb->leader) = new_node;
        return;
    }

    TAC *last = *(bb->leader);
    while (last->next != NULL) {
        last = last->next;
    }
    last->next = new_node;
    return;
}

BB *appendBB(BB **bb, BB *new_node) {
    new_node->next = NULL;

    if ((*bb)->leader == NULL) {
        (*bb)->leader = new_node->leader;
        (*bb)->next = new_node->next;
        return *bb;
    }

    BB *last = *bb;
    while (last->next != NULL) {
        last = last->next;
    }
    last->next = new_node;
    return last->next;
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
    if (term->type == IDENTIFIER)
        store->args.store.value = term->lexeme;
    else {
        char *temp = malloc(100 * sizeof(char));
        snprintf(temp, 100, "%d", term->value);
        store->args.store.value = temp;
    }
    return store;
}

TAC *create_single_TAC_seq(BB *bb) {
    TAC *new_tac = calloc(1, sizeof(TAC));
    BB *bbtemp = bb;

    TAC *temp_new_tac = new_tac;
    while (bbtemp != NULL) {
        TAC *temp = *(bbtemp->leader);
        while (temp != NULL) {
            temp_new_tac->next = temp;

            temp_new_tac = temp_new_tac->next;
            temp = temp->next;
        }
        bbtemp = bbtemp->next;
    }
    return new_tac->next;
}

// Method to make basic block sequence into one big TAC sequence

// Refactor for basic blocks
void printTAC(FILE *file, BB *bb) {
    BB *bbtemp = bb;
    while (bbtemp != NULL) {
        TAC *temp = *(bbtemp->leader);
        while (temp != NULL) {
            switch (temp->op) {
            case PROC_OP:
                fprintf(file, "\nproc %s %d\nblock\n",
                        temp->args.proc.name->lexeme, temp->args.proc.arity);
                break;
            case CALL_OP:
                fprintf(file, "%s = call %s\n", temp->args.call.store,
                        temp->args.call.name->lexeme);
                break;
            case BLOCK_OP:
                // if its proc //endproc if its if //endif

                fprintf(file, "endblock %d\n\n", (temp->args.block.nvars));
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

void printTAC(FILE *file, TAC *seq) {
    TAC *temp = seq;
    while (temp != NULL) {
        switch (temp->op) {
        case FUNCTION:
            fprintf(file, "proc %s ()\n", temp->args.call.name->lexeme);
            break;
        case BLOCK_OP:
            fprintf(file, "endblock %d\n", *(temp->args.block.nvars));
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
        case RETURN:
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
            fprintf(file, "add %s %s %s\n", temp->args.expr.src1,
                    temp->args.expr.src2, temp->args.expr.dst);
            break;
        default:
            break;
        }
        temp = temp->next;
    }
}