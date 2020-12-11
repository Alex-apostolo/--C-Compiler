#include "tac_generator.h"
#include "C.tab.h"
#include "interpreter.h"
#include "nodes.h"
#include <stdio.h>
#include <stdlib.h>

/*Function definitions*/
void append(BB *, TAC *);
BB *appendBB(BB **, BB *);
char *treg_generator();
char *label_generator();
TAC *create_load_TAC(TOKEN *term);
TAC *create_store_TAC(TOKEN *term);

/*Global Variables*/
int ntreg = 0;
char *latest_treg;
int nlabel = 0;
char *latest_label;
int nvars = 0;
char *svars[31];

void *tac_generator(NODE *term, BB **bb) {
    switch (term->type) {
    case LEAF:
        // Left child has value: STRING_LITERAL, IDENTIFIER, CONSTANT
        return tac_generator(term->left, bb);
        break;
    case IDENTIFIER:
    case CONSTANT:
    case STRING_LITERAL:
        // Interpret as TOKEN*
        return term;
        break;
    case APPLY: {
        if (term->right != NULL) {
            tac_generator(term->right, bb);
        }
        // Simply call the function: this will be a ja in MIPS
        TAC *call = calloc(1, sizeof(TAC));
        call->op = CALL_OP;
        call->args.call.name = ((TOKEN *)term->left)->lexeme;
        call->args.call.store = treg_generator();
        call->next = NULL;
        append(*bb, call);
        return latest_treg;
        break;
    }
    case VOID:
    case FUNCTION:
    case INT:
        break;
    case 'd':
        // Left child is an AST for Return type
        tac_generator(term->left, bb);
        // Right child is the Name and arguments
        tac_generator(term->right, bb);
        break;
    case 'D': {
        // Append to Basic Blocks
        BB *new_bb = calloc(1, sizeof(BB));
        TAC **new_seq = calloc(1, sizeof(TAC *));
        new_bb->leader = new_seq;
        new_bb = appendBB(bb, new_bb);
        // Left child is the Function definition
        tac_generator(term->left, &new_bb);
        // Right child is the Function body
        if (term->right != NULL) {
            return tac_generator(term->right, &new_bb);
        } else {
            return 0;
        }
        break;
    }
    case 'F': {
        // Left child is the Name of function
        TAC *func = (TAC *)malloc(sizeof(TAC));
        func->next = NULL;
        func->op = FUNCTION;
        func->args.proc.name = tac_generator(term->left, bb);
        nvars = 0;

        if (term->right != NULL) {
            int varsBefore = nvars;
            tac_generator(term->right, bb);
            func->args.proc.arity = nvars - varsBefore;
        } else {
            func->args.proc.arity = 0;
        }
        append(*bb, func);
        break;
    }
    case CONTINUE:
    case BREAK:
        break;
    case RETURN: {
        // Left child is an AST of the expression whose value is to be returned
        // TODO: handle the case where left child is identifier and the case for
        // everything else
        TAC *ret = (TAC *)malloc(sizeof(TAC));
        ret->next = NULL;
        ret->op = RETURN;
        if (term->left != NULL) {
            if (term->left->type == LEAF) {
                TOKEN *temp = tac_generator(term->left, bb);
                if (term->left->left->type == IDENTIFIER) {
                    ret->args.ret.type = IDENTIFIER;
                    ret->args.ret.val.identifier = temp->lexeme;
                } else if (term->left->left->type == CONSTANT) {
                    ret->args.ret.type = CONSTANT;
                    ret->args.ret.val.constant = temp->value;
                }
            } else {
                // if (term->left->type == APPLY) {
                //     tac_generator(term->left,bb);
                //     ret->args.ret.type = TREG;
                //     ret->args.ret.val.treg = latest_treg;
                // } else {
                ret->args.ret.type = TREG;
                ret->args.ret.val.treg = tac_generator(term->left, bb);
                // }
            }
            append(*bb, ret);
            TAC *block = (TAC *)malloc(sizeof(TAC));
            block->next = NULL;
            block->op = BLOCK_OP; /* This is the OP CODE for BLOCK*/
            // TODO: make it track how many variables have been declared
            // maybe at the end of the return
            block->args.block.nvars = nvars;
            append(*bb, block);
            return bb;
        } else {
            // TODO: throw an error
            printf("Error no return type\n");
        }
        break;
    }
    case ',':
        tac_generator(term->left, bb);
        tac_generator(term->right, bb);
        break;
    case '~':
        if (term->left->type == LEAF) {
            // Left child is the type
            tac_generator(term->left, bb);

            // Right child is the variable name
            if (term->right->type == LEAF) {
                svars[nvars] =
                    ((TOKEN *)tac_generator(term->right, bb))->lexeme;
            } else {
                // Right child is the AST "=" and we declare the variable before
                // assigning
                svars[nvars] =
                    ((TOKEN *)tac_generator(term->right->left, bb))->lexeme;
                tac_generator(term->right, bb);
            }
            nvars++;
        } else {
            tac_generator(term->left, bb);
            tac_generator(term->right, bb);
        }
        break;
    case ';':
        // Left child is the first item or a sequence; returned values are
        // ignored
        tac_generator(term->left, bb);
        // Right child is the last item and also the return value
        return tac_generator(term->right, bb);
        break;
    case '=': {
        // Creates TAC for rhs of expression
        if (term->right->type == LEAF) {
            TAC *rhs = create_load_TAC(tac_generator(term->right, bb));
            append(*bb, rhs);
        } else
            tac_generator(term->right, bb);

        // Creates TAC for lhs of expression
        TAC *lhs = create_store_TAC(tac_generator(term->left, bb));
        append(*bb, lhs);
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
        // CASE WHERE CALLING FUNCTION
        // Creates load TAC instructions
        TAC *src1;
        TAC *src2;
        char *tempr1;
        char *tempr2;
        if (term->left->type != APPLY) {
            src1 = create_load_TAC(tac_generator(term->left, bb));
            append(*bb, src1);
        } else {
            tempr1 = (char *)tac_generator(term->left, bb);
        }
        if (term->right->type != APPLY) {
            src2 = create_load_TAC(tac_generator(term->right, bb));
            append(*bb, src2);
        } else {
            tempr2 = (char *)tac_generator(term->right, bb);
        }

        // Creates oper TAC instruction
        TAC *oper = (TAC *)malloc(sizeof(TAC));
        oper->next = NULL;
        oper->op = term->type;

        if (term->left->type != APPLY) {
            oper->args.expr.src1 = src1->args.load.treg;
        } else {
            oper->args.expr.src1 = tempr1;
        }
        if (term->right->type != APPLY) {
            oper->args.expr.src2 = src2->args.load.treg;
        } else {
            oper->args.expr.src2 = tempr2;
        }
        oper->args.expr.dst = treg_generator();
        append(*bb, oper);
        return oper->args.expr.dst;
        break;
    }
    case IF: {
        NODE *antecedent = term->left;
        NODE *consequent =
            term->right->type == ELSE ? term->right->left : term->right;
        NODE *alternate = term->right->type == ELSE ? term->right->right : NULL;
        // Label generator

        IF_ *if_ = malloc(sizeof(IF_));
        if_->antecedent = ((TOKEN *)term->left)->lexeme;
        if_->label = label_generator();

        // append(*bb->leader, if_);

        break;
    }
    case WHILE:
        break;
    default:
        break;
    }
    return NULL;
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

void append(BB *bb, TAC *new_node) {
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

// Method to make basic block sequence into one big TAC sequence

// Refactor for basic blocks
void printTAC(FILE *file, BB *bb) {
    BB *bbtemp = bb;
    while (bbtemp != NULL) {
        TAC *temp = *(bbtemp->leader);
        while (temp != NULL) {
            switch (temp->op) {
            case FUNCTION:
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
                case '+': fprintf(file, "add"); break;
                case '-': fprintf(file, "sub"); break;
                case '*': fprintf(file, "mul"); break;
                case '/': fprintf(file, "div"); break;
                case '%': fprintf(file, "mod"); break;
                case '>': fprintf(file, "gret"); break;
                case '<': fprintf(file, "less"); break;
                case NE_OP: fprintf(file, "ne"); break;
                case EQ_OP: fprintf(file, "eq"); break;
                case LE_OP: fprintf(file, "eqle"); break;
                case GE_OP: fprintf(file, "eqge"); break;
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