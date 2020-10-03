#include "mpc.h"
#include <stdio.h>
#include <stdlib.h> /* free() */
#include <string.h> /* strcmp() */

typedef struct lval
{
    int type;
    long num;
    /* error and symbol types have some string data */
    char* err;
    char* sym;
    /* count and pointer to a list of "lval*" */
    int count;
    struct lval** cell;
} lval;

enum {LVAL_NUM, LVAL_ERR, LVAL_SYM, LVAL_SEXPR};
// enum {LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM};


#ifdef _WIN32
static char buffer[2048];
/* the fake readline function */
char* readline(char* prompt) {
    fputs(prompt, stdout);
    fgets(buffer, 2048, stdin);
    char* cpy = malloc(strlen(buffer)+1);
    strcpy(cpy, buffer);
    cpy[strlen(cpy)-1] = '\0';
    return cpy;
}

/* the fake add_history function */
void add_history(char* unused){}
#else
#include <editline/readline.h> /* readline() */
#include <editline/history.h>  /* add_history() */
#endif


#define QUIT "quit()"

void lval_expr_print(lval* v, char open, char close);

/* Construct a pointer to a new Number lval */ 
lval* lval_num(long x) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_NUM;
    v->num = x;
    return v;
}

/* Construct a pointer to a new Error lval */ 
lval* lval_err(char* m) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_ERR;
    v->err = malloc(strlen(m) + 1);
    strcpy(v->err, m);
    return v;
}

/* Construct a pointer to a new Symbol lval */ 
lval* lval_sym(char* s) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_SYM;
    v->sym = malloc(strlen(s) + 1);
    strcpy(v->sym, s);
    return v;
}

/* A pointer to a new empty Sexpr lval */
lval* lval_sexpr(void) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_SEXPR;
    v->count = 0;
    v->cell = NULL;
    return v;
}

void lval_del(lval* v) {
    switch (v->type) {
        /* do nothing special for number type */
        case LVAL_NUM: break;

        /* for err or symbol free the string data  */
        case LVAL_ERR: free(v->err); break;
        case LVAL_SYM: free(v->sym); break;

        /* if sexpr then delete all elements inside */
        case LVAL_SEXPR:
            for(size_t i = 0; i < v->count; i++) {
                lval_del(v->cell[i]);
            }
            /* also free the memory allocated to contain the pointers */
            free(v->cell);
        break;
    }
    free(v);
}

lval* lval_read_num(mpc_ast_t* t) {
    errno = 0;
    long x = strtol(t->contents, NULL, 10);
    return errno != ERANGE ? lval_num(x) : lval_err("invalid number!");
}

lval* lval_add(lval* v, lval* x) {
    v->count++;
    v->cell = realloc(v->cell, sizeof(lval*) * v->count);
    v->cell[v->count - 1] = x;
    return v;
}

lval* lval_read(mpc_ast_t* t) {
    /* If Symbol or Number return conversion to that type */
    if(strstr(t->tag, "number")) { return lval_read_num(t); }
    if(strstr(t->tag, "symbol")) { return lval_sym(t->contents); }

    /* if root (>) or sexpr then create empty list */
    lval* x = NULL;
    if(strcmp(t->tag, ">") == 0) { x = lval_sexpr(); }
    if(strcmp(t->tag, "sexpr") == 0) { x = lval_sexpr(); }

    /* fill this list with any valide expression contained within */
    for(size_t i = 0; i < t->children_num; i++) {
        if(strcmp(t->children[i]->contents, "(") == 0) { continue; }
        if(strcmp(t->children[i]->contents, ")") == 0) { continue; }
        if(strcmp(t->children[i]->contents, "{") == 0) { continue; }
        if(strcmp(t->children[i]->contents, "}") == 0) { continue; }
        if(strcmp(t->children[i]->contents, "regex") == 0) { continue; }
        x = lval_add(x, lval_read(t->children[i]));
    }
    return x;
}

void lval_print(lval* v) {
    switch(v->type) {
        case LVAL_NUM: printf("%li", v->num); break;
        case LVAL_ERR: printf("ERROR: %s", v->err); break;
        case LVAL_SYM: printf("%s", v->sym); break;
        case LVAL_SEXPR: lval_expr_print(v, '(', ')'); break;
    }
}

void lval_expr_print(lval* v, char open, char close) {
    putchar(open);
    for(size_t i = 0; i < v->count; i++) {
        lval_print(v->cell[i]);

        /* do not print trailing space if last element */
        if(i != (v->count-1)) {
            putchar(' ');
        }
    }
    putchar(close);
}

void lval_println(lval* v) { lval_print(v); putchar('\n'); }

// --------------------------above is new--------------------------
// void lval_print(lval v) {
//     switch(v.type) {
//         case LVAL_NUM: printf("%li", v.num); break;

//         case LVAL_ERR:
//             if(v.err == LERR_DIV_ZERO) {
//                 printf("Error: Division by zero!");
//             } else if(v.err == LERR_BAD_OP) {
//                 printf("ERROR: Invalid operator!");
//             } else if(v.err == LERR_BAD_NUM) {
//                 printf("ERROR: Invalid number!");
//             }
//             break;
//     }
// }

// void lval_println(lval v) {lval_print(v); putchar('\n');}

// lval eval_op(lval x, char* op, lval y) {
//     if(x.type == LVAL_ERR) { return x; }
//     if(y.type == LVAL_ERR) { return y; }
//     if(strcmp(op, "+") == 0 || strcmp(op, "add") == 0) { return lval_num(x.num + y.num); }
//     if(strcmp(op, "-") == 0 || strcmp(op, "sub") == 0) { return lval_num(x.num - y.num); }
//     if(strcmp(op, "*") == 0 || strcmp(op, "mul") == 0) { return lval_num(x.num * y.num); }
//     if(strcmp(op, "/") == 0 || strcmp(op, "div") == 0) { 
//         return y.num == 0 ? lval_err(LERR_DIV_ZERO) : lval_num(x.num / y.num);
//     }
//     if(strcmp(op, "%") == 0) { 
//         return y.num == 0 ? lval_err(LERR_DIV_ZERO) : lval_num(x.num % y.num);
//     }
// }

// long eval_op(long x, char* op, long y) {
//     if(strcmp(op, "+") == 0 || strcmp(op, "add") == 0) { return x + y; }
//     if(strcmp(op, "-") == 0 || strcmp(op, "sub") == 0) { return x - y; }
//     if(strcmp(op, "*") == 0 || strcmp(op, "mul") == 0) { return x * y; }
//     if(strcmp(op, "/") == 0 || strcmp(op, "div") == 0) { return x / y; }
//     if(strcmp(op, "%") == 0) { return x % y; }
// }

// lval eval(mpc_ast_t* t) {
//     /* if tagged as number return it directly */
//     if (strstr(t->tag, "number")) {
//         errno = 0;
//         long x = strtol(t->contents, NULL, 10);
//         return errno != ERANGE ? lval_num(x) : lval_err(LERR_BAD_NUM);
//     }

//     /* the op is always second child */
//     char* op = t->children[1]->contents;

//     /* store the third child in 'x' */
//     lval x = eval(t->children[2]);

//     /* iterate the remaining children and combining */
//     int i = 3;
//     while(strstr(t->children[i]->tag, "expr")) {
//         x = eval_op(x, op, eval(t->children[i]));
//         i++;
//     }
//     return x;
// }

int main(int argc, char** argv) {
    /* create some parsers */
    mpc_parser_t* Number   = mpc_new("number");
    mpc_parser_t* Symbol   = mpc_new("symbol");
    mpc_parser_t* Sexpr    = mpc_new("sexpr");
    mpc_parser_t* Expr     = mpc_new("expr");
    mpc_parser_t* Jisp     = mpc_new("jisp");

    puts("boring");

    mpca_lang(MPCA_LANG_DEFAULT, 
        "                                                          \
            number: /-?[0-9]+/;                                    \
            symbol: '+' | '-' | '*' | '/' | '%' |                  \
            \"add\" | \"sub\" | \"mul\" | \"div\" |;               \
            sexpr: '(' <expr>* ')';                                \
            expr: <number> | <symbol> | <sexpr>;           \
            jisp: /^/ <operator> <expr>+ /$/;                      \
        ",
    Number, Symbol, Sexpr, Expr, Jisp
    );
    
    puts("\nJisp Version 0.0.0.0.1");
    puts("Press CTRL + c || \"quit()\" to Exit\n");

    while(1) {
        // fputs("Jisp> ", stdout);
        // fgets(input, 2048, stdin);
        char* input = readline(">Jisp ");
        add_history(input);

        if(!strcmp(input, QUIT)) {
            puts("bye!");
            break;
        }

        mpc_result_t r;
        if(mpc_parse("<stdin>", input, Jisp, &r)) {
            /* if success, print the result */
            lval* x = lval_read(r.output);
            lval_println(x);
            lval_del(x);
        } else {
            /* otherwise print the error */
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }
        free(input);
    }

    mpc_cleanup(5, Number, Symbol, Sexpr, Expr, Jisp);

    return 0;
}