/* do some testes */
// #include <stdio.h>
// int main() {
//     char si[] = {'h', 'e', 'l', 'l', 'o'};
//     char* s = "hello";
//     char* str[] = {"c", "python"};

//     printf("%c\n", s[0]);
//     printf("%s\n", str[1]);
//     printf("%s\n", si);
// }
#include <stdio.h>

void foo()
{
    int a = 10;
    static int sa = 10;

    a += 5;
    sa += 5;

    printf("a = %d, sa = %d\n", a, sa);
}
int main()
{
    int i;

    for (i = 0; i < 10; ++i)
        foo();
}