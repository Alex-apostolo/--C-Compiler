#include "mips_generator.h"
#include "C.tab.h"
#include "tac_generator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define OUTPUT "RESULT.s"

AR *activation_record_create(VAR *);
void appendMIPSVAR(VAR **, VAR *);
void _mips_generator(TAC *, FILE *, AR **);

void mips_generator(TAC *seq) {
    if (seq == NULL)
        return;
    // Removes file and opens a new one with the same name for appending
    remove(OUTPUT);
    FILE *file = fopen(OUTPUT, "a");

    if (file == NULL)
        fprintf(stderr, "Error occured trying to create or override file");

    // Add global variables to .data
    fprintf(file, "\t.data\n");
    TAC *temp = seq;
    while (temp->op == GLOBAL_OP) {
        switch (temp->args.glob->type) {
        case INT:
            if (temp->args.glob->val == NULL) {
                fprintf(file, "%s: .word 0\n", temp->args.glob->name);
            } else {
                fprintf(file, "%s: .word %d\n", temp->args.glob->name,
                        temp->args.glob->val->v.integer);
            }
            break;
        case STRING_LITERAL:
            if (temp->args.glob->val == NULL) {
                fprintf(file,
                        "%s: .asciiz "
                        "\n",
                        temp->args.glob->name);
            } else {
                fprintf(file, "%s: .asciiz %s\n", temp->args.glob->name,
                        temp->args.glob->val->v.string);
            }
            break;
            // case BOOLEAN:
        }
        temp = temp->next;
    }
    fprintf(file, "\n\t.text\n\t.globl main\n");

    AR **ar = calloc(1, sizeof(AR *));
    _mips_generator(seq, file, ar);
}

int is_main = 0;

void _mips_generator(TAC *seq, FILE *file, AR **ar) {
    TAC *temp = seq;
    while (temp != NULL) {
        switch (temp->op) {
        case PROC_OP:
            fprintf(file, "\n%s:\n", temp->args.proc->name);
            is_main = strcmp(temp->args.call->name, "main") == 0;

            if (!is_main) {
                callee_print_ar(file, *ar);
            }
            break;
        case CALL_OP:
            *ar = activation_record_create(*(temp->args.block->svars));
            caller_print_ar(file, *ar);
            fprintf(file, "\tjal %s\n", temp->args.call->name);
            break;
        case BLOCK_OP: {
            break;
        }

        case LOAD_OP:
            // change to int the value of load
            if (temp->args.load->type == IDENTIFIER)
                fprintf(file, "\tlw $%s,%s\n", temp->args.load->treg,
                        temp->args.load->val.identifier);
            else
                fprintf(file, "\tli $%s,%s\n", temp->args.load->treg,
                        temp->args.load->val.constant);
            break;
        case STORE_OP:
            fprintf(file, "\tsw $%s,%s\n", temp->args.store->treg,
                    temp->args.store->value);
            break;
        case RET_OP:
            switch (temp->args.ret->type) {
            case IDENTIFIER: {
                // fprintf(file,
                // "\tlw $%s,%s\n\tmove $a0,$%s\n\tli $v0,17\n\tsyscall\n",
                // , temp->args.ret->val.identifier, regis);
                break;
            }
            case CONSTANT:
                fprintf(file, "\tli $a0,%d\n\tli $v0,17\n\tsyscall\n",
                        temp->args.ret->val.constant);
                break;
            case TREG:
                fprintf(file, "\tmove $a0,$%s\n\tli $v0,17\n\tsyscall\n",
                        temp->args.ret->val.treg);
                break;
            }
            // Restore everything from AR
            if (!is_main)
                callee_print_restore_ar(file, *ar);
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
                fprintf(file, "\tadd");
                break;
            case '-':
                fprintf(file, "\tsub");
                break;
            case '*':
                fprintf(file, "\tmul");
                break;
            case '/':
                fprintf(file, "\tdiv");
                break;
            case '%':
                fprintf(file, "\tmod");
                break;
            case '>':
                break;
            case '<':
                break;
            case NE_OP:
                break;
            case EQ_OP:
                break;
            case LE_OP:
                break;
            case GE_OP:
                break;
            }
            fprintf(file, " $%s,$%s,$%s\n", temp->args.expr->dst,
                    temp->args.expr->src1, temp->args.expr->src2);

            break;
        }
        temp = temp->next;
    }
    fclose(file);
}

VAR *mipsvar_create(char *name, int type, VAR *next) {
    VAR *new_var = calloc(1, sizeof(VAR));
    new_var->name = name;
    new_var->type = type;
    new_var->next = next;
    return new_var;
}

void appendMIPSVAR(VAR **seq, VAR *new_node) {
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

// This function is used right before we call a function
AR *activation_record_create(VAR *vars) {
    AR *new_ar = calloc(1, sizeof(AR));
    VAR **params = calloc(1, sizeof(VAR));
    int params_size = 0;
    VAR **locals = calloc(1, sizeof(VAR));
    int locals_size = 0;
    VAR **tregs = calloc(1, sizeof(VAR));
    int tregs_size = 0;

    VAR *temp = vars;
    while (temp != NULL) {
        switch (temp->type) {
        case IDENTIFIER:
            appendMIPSVAR(locals, mipsvar_create(temp->name, IDENTIFIER, NULL));
            locals_size++;
            break;
        case TREG:
            appendMIPSVAR(tregs, mipsvar_create(temp->name, TREG, NULL));
            tregs_size++;
            break;
        case AREG:
            appendMIPSVAR(params, mipsvar_create(temp->name, AREG, NULL));
            params_size++;
            break;
        }
        temp = temp->next;
    }
    new_ar->params = *params;
    new_ar->params_size = 4 * params_size;
    new_ar->locals = *locals;
    new_ar->locals_size = 4 * locals_size;
    new_ar->tregs = *tregs;
    new_ar->tregs_size = 4 * tregs_size;
    return new_ar;
}

void caller_print_ar(FILE *file, AR *ar) {
    // Print the arguments
    VAR *temp = ar->params;
    int params_size = ar->params_size;
    for (int i = 0; (i * 4) < params_size; i++) {
        if (i == 0) {
            fprintf(file, "\t# Push arguments\n\tadd $sp, $sp, -%d\n",
                    params_size);
        }
        fprintf(file, "\tsw $%s, %d($sp)\n", temp->name, (i * 4));
        temp = temp->next;
    }

    // Print the temporaries
    VAR *temp = ar->tregs;
    int tregs_size = ar->tregs_size;
    for (int i = 0; (i * 4) < tregs_size; i++) {
        if (i == 0) {
            fprintf(file, "\t# Push arguments\n\tadd $sp, $sp, -%d\n",
                    tregs_size);
        }
        fprintf(file, "\tsw $%s, %d($sp)\n", temp->name, (i * 4));
        temp = temp->next;
    }
    fprintf(file, "\t # Jump instruction\n");
}

void callee_print_ar(FILE *file, AR *ar) {
    // Push $ra, old $fp to the stack and set new $fp
    fprintf(file, "\t# Push $ra and old $fp\n\tadd $sp, $sp, "
                  "-8\n\tsw $fp, 4($sp)\n\tsw $ra, 0($sp)\n\t# New frame "
                  "pointer\n\tadd $fp, $sp, -8\n");
    // Implement for saved registers $s0 etc
}

void callee_print_restore_ar(FILE *file, AR *ar) {
    // Restore $ra
    fprintf(file, "\t# Restore $ra\n\tlw $ra, 0($fp)\n");

    // Restore temporaries
    VAR *temp = ar->tregs;
    int tregs_size = ar->tregs_size;
    for (int i = 0; (i * 4) < tregs_size; i++) {
        if (i == tregs_size) {
            fprintf(file, "\t# Push arguments\n\tadd $sp, $sp, -%d\n",
                    tregs_size);
        }
        fprintf(file, "\tlw $%s, %d($)\n", temp->name, (i * 4));
        temp = temp->next;
    }
    fprintf(file, "\t # Jump instruction\n");

    // Restore arguments
    VAR *temp = ar->params;
    int params_size = ar->params_size;
    for (int i = 0; (i * 4) < params_size; i++) {
        if (i == 0) {
            fprintf(file, "\t# Push arguments\n\tadd $sp, $sp, -%d\n",
                    params_size);
        }
        fprintf(file, "\tsw $%s, %d($sp)\n", temp->name, (i * 4));
        temp = temp->next;
    }
}