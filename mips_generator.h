#ifndef __MIPS_G
#define __MIPS_G
#include "tac_generator.h"

void mips_generator(TAC *);

typedef struct ar {
    // callers return address is stored at s0
    char *ra;
    // static link
    VAR *params;
    VAR *locals;
    VAR *tregs;
    int size;
}AR;

#endif