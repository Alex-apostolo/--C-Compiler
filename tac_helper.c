#include "tac_generator.h"
char *treg_generator();
char *label_generator();
TAC *create_load_TAC(TOKEN *, ENV_TAC *);
TAC *create_store_TAC(TOKEN *, ENV_TAC *, TAC **);
void appendVAR(VAR **, VAR *, ENV_TAC *);
void append(TAC **, TAC *);

// Integer to Ascii
char *my_itoa(int num) {
    char *str = malloc(100 * sizeof(char));
    sprintf(str, "%d", num);
    return str;
}

// Generator for tregs for a specified environment
char *treg_generator(ENV_TAC *env) {
    // Maximum of 3 characters, since we have 32 registers at our disposal
    char *str = malloc(3 * sizeof(char));
    snprintf(str, sizeof(str), "t%d", env->ntreg++);
    env->latest_treg = str;
    // Append register to svars
    appendVAR(env->svars, var_create(str, TREG, NULL),env);
    return str;
}

// Generator for labels for a specified environment
char *label_generator(ENV_TAC *env) {
    // Maximum of 5 characters, more than enough for the labels of our programs
    char *str = malloc(5 * sizeof(char));
    snprintf(str, sizeof(str), "L%d", env->nlabel++);
    env->latest_label = str;
    return str;
}

// Append a VAR to a sequence, used later for the Activation Record
void appendVAR(VAR **seq, VAR *new_node, ENV_TAC *env) {
    (*env->nvars)++;
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

// Append a new TAC in the sequence
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

// Creates a load TAC
// This is a helper method for making source code less dense
TAC *create_load_TAC(TOKEN *term, ENV_TAC *env) {
    char *treg = treg_generator(env);
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


// Creates a store TAC
// This is a helper method for making source code less dense
TAC *create_store_TAC(TOKEN *term, ENV_TAC *env, TAC **seq) {
    STORE *new_store = store_create(env->latest_treg, NULL);
    if (term->type == IDENTIFIER) {
        new_store = store_create(env->latest_treg, term->lexeme);
    } else {
        char *temp = malloc(100 * sizeof(char));
        snprintf(temp, 100, "%d", term->value);
        new_store = store_create(env->latest_treg, temp);
    }
    return tac_create(STORE_OP, new_store, NULL);
}

// Refactor for basic blocks
// Function prints TAC sequence
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