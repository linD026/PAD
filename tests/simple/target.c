#include <stdio.h>
#include "../../include/uapi/pad.h"

void __pad_trace target(void)
{
    printf("This target function!\n");
}

void __pad_handler handler(void)
{
    printf("this is internal handler\n");
    pad_buitlin_handler();
}

int main(void)
{
    pad_init(handler, 0);

    printf("Please insert the breakpoint...\n");
    getchar();
    printf("breakpoint entering...\n");
    target();
    printf("breakpoint exiting...\n");

    pad_exit();

    return 0;
}
