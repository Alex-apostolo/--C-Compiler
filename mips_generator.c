#include "mips_generator.h"
#include "C.tab.h"
#include "mips_helper.c"
#include "tac_generator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define OUTPUT "RESULT.s"

void _mips_generator(TAC *, FILE *, AR **);

// Open a new file and initialize .data field
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
    // Maximum variables is 100
    int length = 100;
    // Added_variables consists of all the global and local variables that are
    // in use by functions
    char **added_variables = calloc(length, sizeof(char *));

    // If its Global or if its Store add it to .data
    while (temp != NULL) {
        if (temp->op == GLOBAL_OP) {
            // Depending on type declare it differently
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
            add_variable(added_variables, temp->args.glob->name, length);
        }
        if (temp->op == STORE_OP) {
            // If it doesnt exist already add it
            if (add_variable(added_variables, temp->args.store->value,
                             length)) {
                fprintf(file, "%s:\n", temp->args.store->value);
            }
        }
        temp = temp->next;
    }
    fprintf(file, "\n\t.text\n\t.globl main\n");

    // Call recursive _mips_generator function
    AR **ar = calloc(1, sizeof(AR *));
    _mips_generator(seq, file, ar);
}

// Used for defining different behaviour for the main function
int is_main = 0;

void _mips_generator(TAC *seq, FILE *file, AR **ar) {
    TAC *temp = seq;
    while (temp != NULL) {
        switch (temp->op) {
        case PROC_OP:
            fprintf(file, "\n%s:\n", temp->args.proc->name);
            is_main = strcmp(temp->args.call->name, "main") == 0;

            // If its not main, then print the ar for the callee
            if (!is_main) {
                callee_print_ar(file, *ar);
            }
            break;
        case CALL_OP:
            // Create activation record when a call is made
            *ar = activation_record_create(*(temp->args.call->svars));
            // Push temp, args and locals to the stack
            caller_print_ar(file, *ar);
            // Call function    
            fprintf(file, "\tjal %s\n\n", temp->args.call->name);
            break;

        case LOAD_OP:
            // Load value or address
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
            // Restore everything from AR
            if (!is_main && *ar != 0)
                callee_print_restore_ar(file, *ar);

            switch (temp->args.ret->type) {
            // Select appropriate return type
            case IDENTIFIER: {
                // Since we have already saved the registers i can use the first
                // one
                char *treg = "t0";
                fprintf(file, "\tlw $%s, %s\n", treg,
                        temp->args.ret->val.identifier);
                is_main ? fprintf(file,
                                  "\tmove $a0, $%s\n\tli $v0, 17\n\tsyscall\n",
                                  treg)
                        : fprintf(file, "\tmove $v0, $%s\n\tjr $ra\n", treg);
                break;
            }
            case CONSTANT:
                is_main ? fprintf(file, "\tli $a0,%d\n\tli $v0,17\n\tsyscall\n",
                                  temp->args.ret->val.constant)
                        : fprintf(file, "\tli $v0,%d\n\tjr $ra\n",
                                  temp->args.ret->val.constant);
                break;
            case TREG:
                is_main
                    ? fprintf(file, "\tmove $a0,$%s\n\tli $v0,17\n\tsyscall\n",
                              temp->args.ret->val.treg)
                    : fprintf(file, "\tmove $v0,$%s\n\tjr $ra\n",
                              temp->args.ret->val.treg);
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
