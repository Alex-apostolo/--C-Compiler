#include "C.tab.h"
#include "interpreter.h"
#include "mips_generator.h"
#include "nodes.h"
#include "tac_generator.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

char *named(int t) {
    static char b[100];
    if (isgraph(t) || t == ' ') {
        sprintf(b, "%c", t);
        return b;
    }
    switch (t) {
    default:
        return "???";
    case IDENTIFIER:
        return "id";
    case CONSTANT:
        return "constant";
    case STRING_LITERAL:
        return "string";
    case LE_OP:
        return "<=";
    case GE_OP:
        return ">=";
    case EQ_OP:
        return "==";
    case NE_OP:
        return "!=";
    case EXTERN:
        return "extern";
    case AUTO:
        return "auto";
    case INT:
        return "int";
    case VOID:
        return "void";
    case APPLY:
        return "apply";
    case LEAF:
        return "leaf";
    case IF:
        return "if";
    case ELSE:
        return "else";
    case WHILE:
        return "while";
    case CONTINUE:
        return "continue";
    case BREAK:
        return "break";
    case RETURN:
        return "return";
    }
}

void print_leaf(NODE *tree, int level) {
    TOKEN *t = (TOKEN *)tree;
    int i;
    for (i = 0; i < level; i++)
        putchar(' ');
    if (t->type == CONSTANT)
        printf("%d\n", t->value);
    else if (t->type == STRING_LITERAL)
        printf("\"%s\"\n", t->lexeme);
    else if (t)
        puts(t->lexeme);
}

void print_tree0(NODE *tree, int level) {
    int i;
    if (tree == NULL)
        return;
    if (tree->type == LEAF) {
        // printf("%d", tree->type);
        print_leaf(tree->left, level);
    } else {
        for (i = 0; i < level; i++)
            putchar(' ');
        // printf("%d", tree->type);
        printf("%s\n", named(tree->type));
        /*       if (tree->type=='~') { */
        /*         for(i=0; i<level+2; i++) putchar(' '); */
        /*         printf("%p\n", tree->left); */
        /*       } */
        /*       else */
        print_tree0(tree->left, level + 2);
        print_tree0(tree->right, level + 2);
    }
}

void print_tree(NODE *tree) { print_tree0(tree, 0); }

extern int yydebug;
extern NODE *yyparse(void);
extern NODE *ans;
extern void init_symbtable(void);

int main(int argc, char **argv) {
    int interp = false;
    int tac = false;
    int mips = false;
    int print_AST = false;
    int print_bind = false;

    NODE *tree;

    int option;
    while ((option = getopt(argc, argv, "pdistfb")) != -1) {
        switch (option) {
        case 'p':
            init_symbtable();
            printf("--C COMPILER\n");
            yyparse();
            tree = ans;
            printf("parse finished with %p\n", tree);
            print_tree(tree);
            tree = ans;
            ENV *env = calloc(1,sizeof(ENV));
            VALUE *exit_code = interpret(tree, env);
            printf("Terminated with exit code ");
            switch(exit_code->type) {
                case CONSTANT:
                printf("'%d'\n",exit_code->v.integer);
                return exit_code->v.integer;
                break;
                case STRING_LITERAL:
                printf("'%s'\n",exit_code->v.string);
                return exit_code->v.string;
                break;
                case FUNCTION:
                printf("func\n");
                return FUNCTION;
                break;
            }
            break;
        case 'f':
            print_AST = true;
            break;
        case 'd':
            yydebug = 1;
            break;
        case 'i':
            interp = true;
            break;
        case 's':
            mips = true;
            break;
        case 't':
            tac = true;
            break;
        case 'b':
            print_bind = true;
            break;
        default:
            break;
        }
    }

    if ((interp + tac + mips) > 1) {
        fprintf(stderr, "mycc: Flags -i,-t,-s must be mutually exlusive!\n");
        return -1;
    }

    char *filename;
    init_symbtable();

    // Last argument is always the file except if -p is specified for
    // interactive interpreter
    if ((filename = argv[optind]) != NULL) {
        if (access(filename, R_OK) == -1) {
            fprintf(stderr, "mycc: File specified does not exist! Please "
                            "provide an extant file...\n");
            return -1;
        }

        FILE *file = fopen(filename, "r");
        char *singleLine[150];
        // TODO: change it so that the string is dynamic
        // Currently it can hold 50 lines
        char *wholeFile[50 * 150];
        while (!feof(file)) {
            fgets(singleLine, 150, file);
            strcat(wholeFile, singleLine);
        }
        yy_scan_string(wholeFile);
        yyparse();
        tree = ans;
        fclose(file);

        if (print_AST == true) {
            printf("\n\n\n$$$ PARSE TREE $$$\n\nparse finished with %p\n", tree);
            print_tree(tree);
            printf("\n$$$ END OF PARSE TREE $$$");
        }

        if (interp == true) {
            printf("\n\n\n### OUTPUT ###\n\n");
            ENV *env = calloc(1, sizeof(ENV));
            VALUE *exit_code = interpret(tree, env);
            // Free all fields of env
            printf("Terminated with exit code ");
            switch(exit_code->type) {
                case CONSTANT:
                printf("'%d'\n",exit_code->v.integer);
                break;
                case STRING_LITERAL:
                printf("'%s'\n",exit_code->v.string);
                break;
                case FUNCTION:
                printf("func\n");
                break;
            }
            printf("\n### END OF OUTPUT ###\n\n\n");
            // Prints global frame bindings
            if (print_bind == TRUE)
                print_bindings(env->global);
            switch(exit_code->type) {
                case CONSTANT:
                return exit_code->v.integer;
                break;
                case STRING_LITERAL:
                return (int)exit_code->v.string;
                break;
                case FUNCTION:
                return FUNCTION;
                break;
            }
        }

        if (tac == true) {
            tac_generator(tree);
            printf("\n");
            remove("RESULT.tac");
            FILE *tacfile = fopen("RESULT.tac", "a");

            if (tacfile == NULL)
                fprintf(stderr,
                        "Error occured trying to create or override file");
            //printTAC(tacfile, bb);
            // Free all fields of seq
            return 0;
        }

        // Default action is to run the compiler
        if ((interp == false && tac == false) || mips) {
            // Make BB into one long TAC *
            TAC *seq = calloc(1,sizeof(TAC));
            tac_generator(tree);
            //TAC *seq = create_single_TAC_seq(bb);
            mips_generator(seq);
            printf("\n");
            return 0;
        }
    } else {
        fprintf(stderr, "mycc: You need to specify a file to read from, "
                        "otherwise use -p flag for interactive interpreter\n");
        return -1;
    }
}