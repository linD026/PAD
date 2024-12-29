#include <stdio.h>


void breakpoint(void)
{
    printf("This breakpoint function!\n");
}

int main(void)
{
    printf("Please insert the breakpoint...\n");
    getchar();
    printf("breakpoint entering...\n");
    breakpoint();
    printf("breakpoint exiting...\n");

    return 0;
}
