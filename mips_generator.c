#include "tac_generator.h"
#include "C.tab.h"
#include <stdlib.h>

void mips_generator(TAC *seq) {
    if(seq == NULL) return;
    TAC *temp = seq;
    char *mips = malloc(10000*sizeof(char));
    //open file
    //append to file in the while loop
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
}