#include "mips_generator.h"
#include "C.tab.h"
#include "tac_generator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define OUTPUT "RESULT.s"

void printAR(FILE *, AR *);
void print_restoreAR(FILE *, AR *);
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
        switch (temp->args.glob->val->type) {
        case CONSTANT:
            fprintf(file, "%s: .word %d\n", temp->args.glob->name,
                    temp->args.glob->val->v.integer);
            break;
        case STRING_LITERAL:
            fprintf(file, "%s: .asciiz %s\n", temp->args.glob->name,
                    temp->args.glob->val->v.string);
            break;
            // case BOOLEAN:
        }
        temp = temp->next;
    }
    fprintf(file, "\n\t.text\n\t.globl main");

    AR **ar = calloc(1, sizeof(AR *));
    _mips_generator(seq, file, ar);
}

void _mips_generator(TAC *seq, FILE *file, AR **ar) {
    TAC *temp = seq;
    while (temp != NULL) {
        switch (temp->op) {
        case PROC_OP:
            fprintf(file, "\n%s:\n", temp->args.proc->name);
            break;
        case CALL_OP:
            // Copies return address to stack register, jumps to the function,
            // and copies stack register to return address register.
            // We do this to ensure that callee knows which is the return
            // address of the caller.
            fprintf(file, "\n\tmove $s0, $ra\n\tjal %s\n\tmove $ra, $s0\n\n",
                    temp->args.call->name);
            break;
        case BLOCK_OP: {
            // IF ITS NOT MAIN

            // Create an AR
            *ar = activation_record_create(*(temp->args.block->svars));

            // Print pushing everything on AR into stack
            printAR(file, *ar);
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
                char *regis = treg_generator();
                fprintf(file,
                        "\tlw $%s,%s\n\tmove $a0,$%s\n\tli $v0,17\n\tsyscall\n",
                        regis, temp->args.ret->val.identifier, regis);
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
            print_restoreAR(file, *ar);
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

AR *activation_record_create(VAR *vars) {
    AR *new_ar = calloc(1, sizeof(AR));
    VAR **params = calloc(1, sizeof(VAR));
    VAR **locals = calloc(1, sizeof(VAR));
    VAR **tregs = calloc(1, sizeof(VAR));
    int size = 0;

    VAR *temp = vars;
    while (temp != NULL) {
        switch (temp->type) {
        case IDENTIFIER:
            appendMIPSVAR(locals, mipsvar_create(temp->name, IDENTIFIER, NULL));
            size++;
            break;
        case TREG:
            appendMIPSVAR(tregs, mipsvar_create(temp->name, TREG, NULL));
            size++;
            break;
        case AREG:
            appendMIPSVAR(params, mipsvar_create(temp->name, AREG, NULL));
            size++;
            break;
        }
        temp = temp->next;
    }
    new_ar->ra = "$s0";
    size++;
    new_ar->size = 4 * size;
    new_ar->params = *params;
    new_ar->locals = *locals;
    new_ar->tregs = *tregs;
    return new_ar;
}

void printAR(FILE *file, AR *ar) {
    int offset = 0;
    // Print ra
    fprintf(file,
            "\t# Push Caller Activation Record to stack\n\t# "
            "$s0 temporary storage for $ra\n\tsub $sp, "
            "$sp, %d\n\tsw $s0, %d($sp)\n",
            ar->size, offset);
    offset = offset + 4;
    // Print params
    VAR *temp = ar->params;
    while (temp != NULL) {
        fprintf(file, "\tsw $%s, %d($sp)\n", temp->name, offset);
        offset = offset + 4;
        temp = temp->next;
    }
    // Print locals
    temp = ar->locals;
    while (temp != NULL) {
        fprintf(file, "\tsw %s, %d($sp)\n", temp->name, offset);
        offset = offset + 4;
        temp = temp->next;
    }
    // Print tregs
    temp = ar->tregs;
    while (temp != NULL) {
        fprintf(file, "\tsw $%s, %d($sp)\n", temp->name, offset);
        offset = offset + 4;
        temp = temp->next;
    }
    fprintf(file, "\n\t# Function Body starts here\n");
}

void print_restoreAR(FILE *file, AR *ar) {
    int offset = 0;
    // Print ra
    fprintf(file,
            "\t# Function Body ends here\n\n\t# Restore Caller Activation Record from the stack"
            "\n\tlw $s0, %d($sp)\n", offset);
    offset = offset + 4;
    // Print params
    VAR *temp = ar->params;
    while (temp != NULL) {
        fprintf(file, "\tlw $%s, %d($sp)\n", temp->name, offset);
        offset = offset + 4;
        temp = temp->next;
    }
    // Print locals
    temp = ar->locals;
    while (temp != NULL) {
        fprintf(file, "\tlw %s, %d($sp)\n", temp->name, offset);
        offset = offset + 4;
        temp = temp->next;
    }
    // Print tregs
    temp = ar->tregs;
    while (temp != NULL) {
        fprintf(file, "\tlw $%s, %d($sp)\n", temp->name, offset);
        offset = offset + 4;
        temp = temp->next;
    }
    fprintf(file, "\tadd $sp, $sp, %d\n", ar->size);
}