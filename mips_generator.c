#include "tac_generator.h"
#include "C.tab.h"
#include <stdlib.h>
#include <stdio.h>
#define OUTPUT "mips.s"

void mips_generator(TAC *seq) {
    if(seq == NULL) return;
    //Removes file and opens a new one with the same name for appending
    remove(OUTPUT);
    FILE *file = fopen(OUTPUT,"a");

    if(file == NULL) fprintf(stderr,"Error occured trying to create or override file");

    fprintf(file,"\t.globl main\n\t.text\nmain:");
    
    TAC *temp = seq;
    while(temp->next != NULL) {
        switch(temp->op) {
            case FUNCTION: 
            break;
            case BLOCK_OP:
            break;
            case LOAD_OP:
            break;
            case STORE_OP:
            break;
            case RETURN:
            break;
            case '+': case '-': case '*': case '/': case '%': case '>': case '<': case NE_OP: case EQ_OP: case LE_OP: case GE_OP:
            break;
        }
        temp = temp->next;
    }
    fclose(file);
}