#include "C.tab.h"
#include "tac_generator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define OUTPUT "RESULT.s"

void mips_generator(TAC *seq) {
    if (seq == NULL)
        return;
    // Removes file and opens a new one with the same name for appending
    remove(OUTPUT);
    FILE *file = fopen(OUTPUT, "a");

    if (file == NULL)
        fprintf(stderr, "Error occured trying to create or override file");

    TAC *temp = seq;
    while (temp != NULL) {
        switch (temp->op) {
        case PROC_OP:
            // Method for adding variables and context into stack
            // PushAR
            // Checks if declaration of process is "main"
            // If it's "main" it sets "main" as .globl
            strcmp(temp->args.proc.name, "main")
                ? fprintf(file, "\n%s:\n", temp->args.proc.name)
                : fprintf(file, "\t.globl main\n\t.text\nmain:\n");
            break;
        case CALL_OP:
            // Create Activation Record and extend it like the frames
            fprintf(file, "\n\tmove $s0, $ar\n\tjal %s\n\tmove $ra, $s0\n\n",
                    temp->args.call.name);
            break;
        case BLOCK_OP: {
            // fprintf(file, "\t.data\n");
            VAR *current = temp->args.block.svars;
            // Declare and don't set, check the type
            for (int i = 0; i < *(temp->args.block.nvars); i++) {
                fprintf(file, "%s: \n", current->name);
                current = current->next;
            }
            break;
        }

        case LOAD_OP:
            // change to int the value of load
            if (temp->args.load.type == IDENTIFIER)
                fprintf(file, "\tlw $%s,%s\n", temp->args.load.treg,
                        temp->args.load.val.identifier);
            else
                fprintf(file, "\tli $%s,%s\n", temp->args.load.treg,
                        temp->args.load.val.constant);
            break;
        case STORE_OP:
            fprintf(file, "\tsw $%s,%s\n", temp->args.store.treg,
                    temp->args.store.value);
            break;
        case RETURN:
            switch (temp->args.ret.type) {
            case IDENTIFIER: {
                char *regis = treg_generator();
                fprintf(file,
                        "\tlw $%s,%s\n\tmove $a0,$%s\n\tli $v0,17\n\tsyscall\n",
                        regis, temp->args.ret.val.identifier, regis);
                break;
            }
            case CONSTANT:
                fprintf(file, "\tli $a0,%d\n\tli $v0,17\n\tsyscall\n",
                        temp->args.ret.val.constant);
                break;
            case TREG:
                fprintf(file, "\tmove $a0,$%s\n\tli $v0,17\n\tsyscall\n",
                        temp->args.ret.val.treg);
                break;
            }
            // append li $a0,53 li $v0,17 syscall
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
            fprintf(file, " $%s,$%s,$%s\n", temp->args.expr.dst,
                    temp->args.expr.src1, temp->args.expr.src2);

            break;
        }
        temp = temp->next;
    }
    fclose(file);
}