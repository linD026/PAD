#include "../printut.h"
#include "../../include/uapi/pad.h"

#define LIST_SIZE 4

static __pad_handler void handler(void)
{
    printf("handler function called\n");
    pad_builtin_handler();
}

#define DEFINE_TARGET(__n)                                \
    static PAD_ENTER_POINT(breakpoint_##__n)              \
    {                                                     \
        pr_info("breakpoint #%s fuction called\n", #__n); \
    }                                                     \
                                                          \
    static __pad_trace void target_##__n(void)            \
    {                                                     \
        pr_info("target #%s function called\n", #__n);    \
    }

DEFINE_TARGET(1)
DEFINE_TARGET(2)
DEFINE_TARGET(3)
DEFINE_TARGET(4)

#define NAME_OF_TARGET(__n) target_##__n

typedef void (*target_t)(void);

target_t targets[LIST_SIZE] = {
    NAME_OF_TARGET(1),
    NAME_OF_TARGET(2),
    NAME_OF_TARGET(3),
    NAME_OF_TARGET(4),
};

#define NAME_OF_BREAKPOINT(__n) breakpoint_##__n

pad_handler_t breakpoints[LIST_SIZE] = {
    NAME_OF_BREAKPOINT(1),
    NAME_OF_BREAKPOINT(2),
    NAME_OF_BREAKPOINT(3),
    NAME_OF_BREAKPOINT(4),
};

int test_register_unregister(void)
{
    struct pad_probe list[LIST_SIZE] = { 0 };

    for (int i = 0; i < LIST_SIZE; i++) {
        list[i].address = (unsigned long)targets[i];
        list[i].breakpoint = (unsigned long)breakpoints[i];

        pad_register_probe(&list[i]);
        targets[i]();
        pad_unregister_probe(&list[i]);
        targets[i]();
    }

    return 0;
}

int test_mult_register(void)
{
    struct pad_probe list[LIST_SIZE] = { 0 };

    for (int i = 0; i < LIST_SIZE; i++) {
        list[i].address = (unsigned long)targets[0];
        list[i].breakpoint = (unsigned long)breakpoints[i];

        pad_register_probe(&list[i]);
    }

    targets[0]();

    for (int i = 0; i < LIST_SIZE; i++) {
        pad_unregister_probe(&list[i]);
    }

    return 0;
}

int main(void)
{
    pad_init(handler, 0);
    UNIT_BUG_ON(test_register_unregister());
    UNIT_BUG_ON(test_mult_register());
    pad_exit();

    return 0;
}
