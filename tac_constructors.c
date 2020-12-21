#include "tac_generator.h"

/*
 * Constructors for 
 * EXPR, 
 * PROC, 
 * CALL, 
 * VAR, 
 * LOAD,
 * STORE,
 * RET,
 * IF_,
 * GOTO,
 * LABEL,
 * VAL,
 * GLOBAL,
 * CLOS,
 * TAC,
 * BB,
 * ENV_TAC
 * 
*/ 

EXPR *expr_create(char *src1, char *src2, char *dst) {
    EXPR *new_expr = calloc(1, sizeof(EXPR));
    new_expr->src1 = src1;
    new_expr->src2 = src2;
    new_expr->dst = dst;
    return new_expr;
}

PROC *proc_create(char *name, int arity) {
    PROC *new_proc = calloc(1, sizeof(PROC));
    new_proc->name = name;
    new_proc->arity = arity;
    return new_proc;
}

CALL *call_create(char *name, int arity, char *store, int *nvars, VAR **svars) {
    CALL *new_call = calloc(1, sizeof(CALL));
    new_call->name = name;
    new_call->arity = arity;
    new_call->store = store;
    new_call->nvars = nvars;
    new_call->svars = svars;
    return new_call;
}

VAR *var_create(char *name,int type, VAR *next) {
    VAR *new_var = calloc(1, sizeof(VAR));
    new_var->name = name;
    new_var->type = type;
    new_var->next = next;
    return new_var;
}

LOAD *load_create(char *treg, int type, void *val) {
    LOAD *new_load = calloc(1, sizeof(LOAD));
    new_load->treg = treg;
    new_load->type = type;
    switch (type) {
    case IDENTIFIER:
        new_load->val.identifier = (char *)val;
        break;
    case CONSTANT:
        new_load->val.constant = (char *)val;
        break;
    }
    return new_load;
}

STORE *store_create(char *treg, char *value) {
    STORE *new_store = calloc(1, sizeof(STORE));
    new_store->treg = treg;
    new_store->value = value;
    return new_store;
}

RET *ret_create(int type, void *val) {
    RET *new_ret = calloc(1, sizeof(RET));
    new_ret->type = type;
    switch (type) {
    case IDENTIFIER:
        new_ret->val.identifier = (char *)val;
        break;
    case CONSTANT:
        new_ret->val.constant = (int)val;
        break;
    case TREG:
        new_ret->val.treg = (char *)val;
        break;
    }
    return new_ret;
}

IF_ *if_create(char *antecedent, char *label) {
    IF_ *new_if = calloc(1, sizeof(IF_));
    new_if->antecedent = antecedent;
    new_if->label = label;
    return new_if;
}

GOTO *goto_create(char *label) {
    GOTO *new_goto = calloc(1, sizeof(GOTO));
    new_goto->label = label;
    return new_goto;
}

LABEL *label_create(char *name) {
    LABEL *new_label = calloc(1, sizeof(LABEL));
    new_label->name = name;
    return new_label;
}

VAL *val_create(int type, void *v) {
    VAL *new_val = calloc(1, sizeof(VAL));
    new_val->type = type;
    switch (type) {
    case CONSTANT:
        new_val->v.integer = (int)v;
        break;
    // case BOOLEAN
    case STRING_LITERAL:
        new_val->v.string = (char *)v;
        break;
    }
    return new_val;
}

GLOBAL *global_create(char *name, int type, VAL *val) {
    GLOBAL *new_global = calloc(1, sizeof(GLOBAL));
    new_global->name = name;
    new_global->type = type;
    new_global->val = val;
    return new_global;
}

CLOS *clos_create(NODE *body, char *name) {
    CLOS *new_clos = calloc(1, sizeof(CLOS));
    new_clos->body = body;
    new_clos->name = name;
    return new_clos;
}

TAC *tac_create(int op, void *args, TAC *next) {
    TAC *new_tac = calloc(1, sizeof(TAC));
    new_tac->op = op;
    new_tac->next = next;
    switch (op) {
    case CALL_OP:
        new_tac->args.call = (CALL *)args;
        break;
    case LOAD_OP:
        new_tac->args.load = (LOAD *)args;
        break;
    case PROC_OP:
        new_tac->args.proc = (PROC *)args;
        break;
    case CLOS_OP:
        new_tac->args.clos = (CLOS *)args;
        break;
    case GLOBAL_OP:
        new_tac->args.glob = (GLOBAL *)args;
        break;
    case STORE_OP:
        new_tac->args.store = (STORE *)args;
        break;
    case RET_OP:
        new_tac->args.ret = (RET *)args;
        break;
    case IF_OP:
        new_tac->args.if_ = (IF_ *)args;
        break;
    case GOTO_OP:
        new_tac->args.goto_ = (GOTO *)args;
        break;
    case LABEL_OP:
        new_tac->args.label = (LABEL *)args;
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
        new_tac->args.expr = (EXPR *)args;
        break;
    }
    return new_tac;
}

BB *bb_create(TAC **leader, BB *next) {
    BB *new_bb = calloc(1, sizeof(BB));
    new_bb->leader = leader;
    new_bb->next = next;
    return new_bb;
}

ENV_TAC *env_tac_create_empty() {
    ENV_TAC *new_env = calloc(1,sizeof(ENV_TAC));
    new_env->ntreg = 0;
    new_env->latest_treg = "";
    new_env->nlabel = 0;
    new_env->latest_treg = "";
    new_env->arguments = 0;
    new_env->svars = calloc(1, sizeof(VAR *));
    new_env->nvars = calloc(1, sizeof(int));
    return new_env;
}

ENV_TAC *env_tac_clear(ENV_TAC *env) {
    env->ntreg = 0;
    env->latest_treg = "";
    env->nlabel = 0;
    env->latest_treg = "";
    env->arguments = 0;
    env->svars = calloc(1, sizeof(VAR *));
    env->nvars = calloc(1, sizeof(int));
    return env;
}