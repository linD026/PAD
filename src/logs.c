#include <pad/logs.h>

#include <stdlib.h>
#include <execinfo.h>

#define STACK_BUF_SIZE 32

void pad_dump_stack(void)
{
    char **stack_info;
    int nr = 0;
    void *buf[STACK_BUF_SIZE];

    nr = backtrace(buf, STACK_BUF_SIZE);
    stack_info = backtrace_symbols(buf, nr);

    pr_err("========== dump stack start ==========\n");
    for (int i = 0; i < nr; i++)
        pr_err("  %s\n", stack_info[i]);
    pr_err("========== dump stack  end  ==========\n");
}

void __pad_exit(int exit_code)
{
    exit(exit_code);
}
