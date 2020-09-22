#include <stdio.h>
#include <stdlib.h> /* free() */
#include <string.h> /* strcmp() */
#include "mpc.h"
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
        puts("Yeah! You are right!");
        free(input);
    }

    return 0;
}

