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

int main(int argc, char** argv) {
    /* create some parsers */
    mpc_parser_t* Number   = mpc_new("number");
    mpc_parser_t* Operator = mpc_new("operator");
    mpc_parser_t* Expr     = mpc_new("expr");
    mpc_parser_t* Jisp     = mpc_new("jisp");

    puts("boring");

    mpca_lang(MPCA_LANG_DEFAULT, 
        "                                                \
            number: /-?[0-9]+/;                          \
            operator: '+' | '-' | '*' | '/';             \
            expr: <number> | '(' <operator> <expr>+ ')'; \
            jisp: /^/ <operator> <expr>+ /$/;            \
        ",
    Number, Operator, Expr, Jisp
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
            /* if success print the AST */
            mpc_ast_print(r.output);
            mpc_ast_delete(r.output);
        } else {
            /* otherwise print the error */
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }
        free(input);
    }

    mpc_cleanup(4, Number, Operator, Expr, Jisp);

    return 0;
}