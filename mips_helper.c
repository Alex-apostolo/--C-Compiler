#include "mips_generator.h"
#include <string.h>
void caller_print_ar(FILE *, AR *);
void callee_print_ar(FILE *, AR *);
void callee_print_restore_ar(FILE *, AR *);
int add_variable(char **, char *, int);
char *mips_treg_generator();
AR *activation_record_create(VAR *);
void appendMIPSVAR(VAR **, VAR *);
VAR *mipsvar_create(char *, int, VAR *);

// Used by the activation record
int ntreg = 0;
char *latest_treg;

// This function is used right before we call a function
// Constructor for activation record
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
            // Appends to list of VARS
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
    new_ar->stack_size = new_ar->params_size + new_ar->locals_size + new_ar->tregs_size + 8; 
    return new_ar;
}

// Prints arguments, tmeporaries and locals of the caller
// This follows closely the calling conventions of gcc
void caller_print_ar(FILE *file, AR *ar) {
    fprintf(file, "\n");
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
    temp = ar->tregs;
    int tregs_size = ar->tregs_size;
    for (int i = 0; (i * 4) < tregs_size; i++) {
        if (i == 0) {
            fprintf(file, "\t# Push t registers\n\tadd $sp, $sp, -%d\n",
                    tregs_size);
        }
        fprintf(file, "\tsw $%s, %d($sp)\n", temp->name, (i * 4));
        temp = temp->next;
    }

    // Print the locals
    temp = ar->locals;
    int locals_size = ar->locals_size;
    for (int i = 0; (i * 4) < locals_size; i++) {
        if (i == 0) {
            fprintf(file, "\t# Push locals\n\tadd $sp, $sp, -%d\n",
                    locals_size);
        }
        fprintf(file, "\tlw $%s, %s\n\tsw $%s, %d($sp)\n",
                mips_treg_generator(), temp->name, latest_treg, (i * 4));
        temp = temp->next;
    }
    ntreg = 0;
    fprintf(file, "\t # Jump instruction\n");
}

// Printed at the start of a called function
void callee_print_ar(FILE *file, AR *ar) {
    // Push $ra, old $fp to the stack and set new $fp
    fprintf(file, "\t# Push $ra and old $fp\n\tadd $sp, $sp, "
                  "-8\n\tsw $ra, 0($sp)\n\tsw $fp, 4($sp)\n\t# New frame "
                  "pointer\n\tadd $fp, $sp, 8\n\n");
    // Implement for saved registers $s0 etc
}

// Print at the end of a called function
// Restores all the saved registers and memory addresses from the stack
// following gcc calling convention
void callee_print_restore_ar(FILE *file, AR *ar) {

    fprintf(file, "\n");
    int offset = 0;

    // Restore locals
    VAR *temp = ar->locals;
    int locals_size = ar->locals_size;
    for (int i = 0; (i * 4) < locals_size; i++) {
        if (i == 0) {
            fprintf(file, "\t# Restore locals\n");
        }
        fprintf(file, "\tlw $%s, %d($fp)\n\tsw $%s, %s\n",
                mips_treg_generator(), (offset * 4), latest_treg, temp->name);
        offset++;
        temp = temp->next;
    }

    // Restore temporaries
    temp = ar->tregs;
    int tregs_size = ar->tregs_size;
    for (int i = 0; (i * 4) < tregs_size; i++) {
        if (i == 0) {
            fprintf(file, "\t# Restore t registers\n");
        }
        fprintf(file, "\tlw $%s, %d($fp)\n", temp->name, (offset * 4));
        offset++;
        temp = temp->next;
    }

    // Restore arguments
    temp = ar->params;
    int params_size = ar->params_size;
    for (int i = 0; (i * 4) < params_size; i++) {
        if (i == 0) {
            fprintf(file, "\t# Push arguments\n\tadd $sp, $sp, -%d\n",
                    params_size);
        }
        fprintf(file, "\tsw $%s, %d($sp)\n", temp->name, (offset * 4));
        offset++;
        temp = temp->next;
    }

    // Restore $ra
    fprintf(file, "\t# Restore $ra\n\tlw $ra, -8($fp)\n");
    // Restore $fp
    fprintf(file, "\t# Restore $fp\n\tlw $fp, -4($fp)\n");
    // Restore $sp
    fprintf(file, "\t# Restore $sp\n\tadd $sp, $sp, %d\n",ar->stack_size);

    fprintf(file, "\t # Jump instruction\n");
}

// This function checks if the variable in check exists in the array of
// variables Returns 1 if successfully added to the array
int add_variable(char **added_variables, char *new_string, int length) {
    char *temp = *added_variables;
    for (int i = 0; i < length - 1; i++) {
        if (added_variables[i] == NULL) {
            added_variables[i] = new_string;
            return 1;
        }
        if (strcmp(added_variables[i], new_string) == 0) {
            return 0;
        }
    }
    return 0;
}

// Same as appendVAR but to be used by the mips function
// difference is that it doesnt accept an environment
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

// Constructor for VAR
VAR *mipsvar_create(char *name, int type, VAR *next) {
    VAR *new_var = calloc(1, sizeof(VAR));
    new_var->name = name;
    new_var->type = type;
    new_var->next = next;
    return new_var;
}

// Same as treg_generator but for mips
// difference is that it doesnt accept an environment
char *mips_treg_generator() {
    // Maximum of 3 characters, since we have 32 registers at our disposal
    char *str = malloc(3 * sizeof(char));
    snprintf(str, sizeof(str), "t%d", ntreg++);
    latest_treg = str;
    // Append register to svars
    return str;
}