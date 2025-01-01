#include <stdio.h>
#include "../../include/uapi/pad.h"

static void __pad_trace my_function(void)
{
    printf("This is my function!\n");
}

static void __pad_handler handler(void)
{
    printf("This is internal handler!\n");
    pad_builtin_handler();
}

int main(void)
{
    pad_init(handler, 0);

    my_function();
    //pad_test_inject((unsigned long)my_function, (unsigned long) handler);
    pad_test_inject((unsigned long)my_function, 0);
    my_function();
    pad_test_recover((unsigned long)my_function);
    my_function();

    pad_exit();

    return 0;
}
