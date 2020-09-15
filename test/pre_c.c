#include <stdio.h>
#include <stdlib.h> /* free() */
#include <string.h> /* strcmp() */
#include <editline/readline.h> /* readline() */
#include <editline/history.h>  /* add_history() */

// static char input[2048];

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
        puts("Yes!You are right!");
        free(input);
    }

    return 0;
}