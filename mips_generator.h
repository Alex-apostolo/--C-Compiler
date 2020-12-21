#ifndef __MIPS_G
#define __MIPS_G
#include "tac_generator.h"

void mips_generator(TAC *);

typedef struct ar {
    char *fp;
    VAR *params;
    int params_size;
    VAR *locals;
    int locals_size;
    VAR *tregs;
    int tregs_size;
    char *ra;
    int stack_size;
}AR;

#endif