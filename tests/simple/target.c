#include <stdio.h>
#include <unistd.h>
#include "../../include/uapi/pad.h"

void __pad_trace target(void)
{
    printf("This target function!\n");
}

void __pad_handler handler(void)
{
    printf("this is internal handler\n");
    pad_builtin_handler();
}

int main(void)
{
    pad_init(handler, PAD_SET_SHMEM_FLAG);

    printf("pid: %d\n", getpid());
    printf("Please insert the breakpoint...\n");
    getchar();
    printf("breakpoint entering...\n");
    target();
    printf("breakpoint exiting...\n");

    pad_exit();

    return 0;
}
