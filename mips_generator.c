#include "C.tab.h"
#include "tac_generator.h"
#include <stdio.h>
#include <stdlib.h>
#define OUTPUT "mips.s"

void mips_generator(TAC *seq) {
    if (seq == NULL)
        return;
    // Removes file and opens a new one with the same name for appending
    remove(OUTPUT);
    FILE *file = fopen(OUTPUT, "a");

    if (file == NULL)
        fprintf(stderr, "Error occured trying to create or override file");

    fprintf(file, "\t.globl main\n\t.text\nmain:\n");

    TAC *temp = seq;
    while (temp != NULL) {
        switch (temp->op) {
        case FUNCTION:
            break;
        case BLOCK_OP:
            break;
        case LOAD_OP:
            //change to int the value of load
            fprintf(file,"\tli $%s,%s\n",temp->args.load.treg, temp->args.load.value);
            break;
        case STORE_OP:
            break;
        case RETURN:
            switch (temp->args.ret.type) {
            case IDENTIFIER:
                //printf("return %s\n", temp->args.ret.val.identifier);
                break;
            case CONSTANT:
                fprintf(file,"\tli $a0,%d\n\tli $v0,17\n\tsyscall\n",temp->args.ret.val.constant);
                break;
            case TREG:
                fprintf(file,"\tmove $a0,$%s\n\tli $v0,17\n\tsyscall\n", temp->args.ret.val.treg);
                break;
            }
            // append li $a0,53 li $v0,17 syscall
            break;
        case '+':
            fprintf(file,"\tadd $%s,$%s,$%s\n",temp->args.expr.dst, temp->args.expr.src1, temp->args.expr.src2);
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
            break;
        }
        temp = temp->next;
    }
    fclose(file);
}