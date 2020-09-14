#include <stdio.h>
#include <string.h>
static char input[2048];

#define QUIT "quit()\n"

int main(int argc, char** argv) {
    puts("\nJisp Version 0.0.0.0.1");
    puts("Press CTRL + c || \"quit()\" to Exit\n");
    while(1) {
        fputs("Jisp> ", stdout);
        fgets(input, 2048, stdin);
        
        if(!strcmp(input, QUIT)) {
            puts("bye!");
            break;
        }
        puts("Yes!You are right!");
    }

    return 0;
}