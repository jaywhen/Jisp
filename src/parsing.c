#include "mpc.h"
#include <stdio.h>
#include <stdlib.h> /* free() */
#include <string.h> /* strcmp() */

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
struct lval;
struct lenv;
typedef lval lval;
typedef lenv lenv;

enum {LVAL_NUM, LVAL_ERR, LVAL_SYM, LVAL_SEXPR, LVAL_FUN, LVAL_QEXPR};
typedef lval* (*lbuiltin)(lenv*, lval*);

typedef struct lval
{
    int type;
    long num;
    /* error and symbol types have some string data */
    char* err;
    char* sym;
    /* count and pointer to a list of "lval*" */
    int count;

    lbuiltin fun;
    lval** cell;
} lval;



void lval_print(lval* v);
lval* lval_eval(lval* v);

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
lval* lval_sexpr() {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_SEXPR;
    v->count = 0;
    v->cell = NULL;
    return v;
}

/* A pointer to a new empty Qexpr lval */
lval* lval_qexpr() {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_QEXPR;
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

        /* if sexpr or qexpr  then delete all elements inside */
        case LVAL_QEXPR:
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

lval* lval_add(lval* v, lval* x) {
    v->count++;
    v->cell = realloc(v->cell, sizeof(lval*) * v->count);
    v->cell[v->count - 1] = x;
    return v;
}

lval* lval_read_num(mpc_ast_t* t) {
    errno = 0;
    long x = strtol(t->contents, NULL, 10);
    return errno != ERANGE ? lval_num(x) : lval_err("invalid number!");
}

lval* lval_read(mpc_ast_t* t) {
    /* If Symbol or Number return conversion to that type */
    if(strstr(t->tag, "number")) { return lval_read_num(t); }
    if(strstr(t->tag, "symbol")) { return lval_sym(t->contents); }

    /* if root (>) or sexpr then create empty list */
    lval* x = NULL;
    if(strcmp(t->tag, ">") == 0) { x = lval_sexpr(); }
    if(strstr(t->tag, "sexpr")) { x = lval_sexpr(); }
    if(strstr(t->tag, "qexpr")) { x = lval_qexpr(); }

    /* fill this list with any valide expression contained within */
    for(size_t i = 0; i < t->children_num; i++) {
        if(strcmp(t->children[i]->contents, "(") == 0) { continue; }
        if(strcmp(t->children[i]->contents, ")") == 0) { continue; }
        if(strcmp(t->children[i]->contents, "{") == 0) { continue; }
        if(strcmp(t->children[i]->contents, "}") == 0) { continue; }
        if(strcmp(t->children[i]->tag, "regex") == 0) { continue; }
        x = lval_add(x, lval_read(t->children[i]));
    }
    return x;
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

void lval_print(lval* v) {
    switch(v->type) {
        case LVAL_NUM: printf("%li", v->num); break;
        case LVAL_ERR: printf("ERROR: %s", v->err); break;
        case LVAL_SYM: printf("%s", v->sym); break;
        case LVAL_SEXPR: lval_expr_print(v, '(', ')'); break;
        case LVAL_QEXPR: lval_expr_print(v, '{', '}'); break;

    }
}

void lval_println(lval* v) { lval_print(v); putchar('\n'); }

/* tools func */
lval* lval_pop(lval* v, int i) {
    /* find the item at "i" */
    lval* x = v->cell[i];

    /* shift memory after the item at "i" over the top */
    memmove(&v->cell[i], &v->cell[i+1], sizeof(lval*) * (v->count-i-1));

    /* decrease the count of items in the list */
    v->count--;

    /* reallocate the memory used */
    v->cell = realloc(v->cell, sizeof(lval*) * v->count);
    return x;
}

lval* lval_take(lval* v, int i) {
    lval* x = lval_pop(v, i);
    lval_del(v);
    return x;
}

lval* builtin_op(lval* a, char* op) {
    /* ensure all args are numbers */
    for(size_t i = 0; i < a->count; i++) {
        if(a->cell[i]->type != LVAL_NUM) {
            lval_del(a);
            return lval_err("cannot operate on non-number!");
        }
    }

    /* pop the first element */
    lval* x = lval_pop(a, 0);

    /* if no args and sub then perform unary negation */
    if((strcmp(op, "-") == 0) && a->count == 0) {
        x->num = -x->num;
    }

    /* while there are still elements remaining */
    while(a->count > 0) {
        /* pop the next element */
        lval* y = lval_pop(a, 0);

        if(strcmp(op, "+") == 0) { x->num += y->num; }
        if(strcmp(op, "-") == 0) { x->num -= y->num; }
        if(strcmp(op, "*") == 0) { x->num *= y->num; }
        if(strcmp(op, "/") == 0) { 
            if(y->num == 0) {
                lval_del(x);
                lval_del(y);
                x = lval_err("Division by zero!"); break;
            }
            x->num /= y->num;
        }
        lval_del(y);
    }
    lval_del(a);
    return x;
}

lval* lval_eval_sexpr(lval* v) {
    for(size_t i = 0; i < v->count; i++) {
        v->cell[i] = lval_eval(v->cell[i]);
    }

    /* error checking */
    for(size_t i = 0; i < v->count; i++) {
        if(v->cell[i]->type == LVAL_ERR) { return lval_take(v, i); }
    }

    /* empty expression */
    if(v->count == 0) { return v; }

    /* single expression */
    if(v->count == 1) { return lval_take(v, 0); }

    /* ensure first element is symbol */
    lval* f = lval_pop(v, 0);
    if(f->type != LVAL_SYM) {
        lval_del(f);
        lval_del(v);
        return lval_err("S-expression dose not start with symbol!");
    }

    /* call builtin with operator */
    lval* result = builtin_op(v, f->sym);
    lval_del(f);
    return result;
}


lval* lval_eval(lval* v) {
    /* evaluate sexpressions */
    if(v->type == LVAL_SEXPR) { return lval_eval_sexpr(v); }
    /* all other lval types remain the same */
    return v;
}

int main(int argc, char** argv) {
    /* create some parsers */
    mpc_parser_t* Number   = mpc_new("number");
    mpc_parser_t* Symbol   = mpc_new("symbol");
    mpc_parser_t* Sexpr    = mpc_new("sexpr");
    mpc_parser_t* Qexpr    = mpc_new("qexpr");
    mpc_parser_t* Expr     = mpc_new("expr");
    mpc_parser_t* Jisp     = mpc_new("jisp");

    

    mpca_lang(MPCA_LANG_DEFAULT, 
        "                                                          \
            number: /-?[0-9]+/;                                    \
            symbol : /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&]+/ ;            \
            sexpr: '(' <expr>* ')';                                \
            qexpr: '{' <expr>* '}';                                \
            expr: <number> | <symbol> | <sexpr> | <qexpr>;         \
            jisp: /^/ <expr>* /$/;                                 \
        ",
    Number, Symbol, Sexpr, Qexpr, Expr, Jisp
    );
    
    puts("\nJisp Version 0.0.0.0.1");
    puts("Press CTRL + c || \"quit()\" to Exit\n");

    while(1) {
        char* input = readline(">Jisp ");
        add_history(input);

        if(!strcmp(input, QUIT)) {
            puts("bye!");
            break;
        }

        mpc_result_t r;
        if(mpc_parse("<stdin>", input, Jisp, &r)) {
            
            /* if success, print the result */
            lval* x = lval_eval(lval_read(r.output));
            lval_println(x);
            lval_del(x);
        } else {
            /* otherwise print the error */
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }
        free(input);
    }

    mpc_cleanup(5, Number, Symbol, Sexpr, Qexpr, Expr, Jisp);

    return 0;
}