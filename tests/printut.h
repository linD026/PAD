#ifndef __PRINTUT_H__
#define __PRINTUT_H__

#include <stdio.h>
#include <stdlib.h>
#include <execinfo.h>

#ifndef likely
#define likely(x) __builtin_expect(!!(x), 1)
#endif

#ifndef unlikely
#define unlikely(x) __builtin_expect(!!(x), 0)
#endif

#define logger_out_stream stdout
#define logger_err_stream stderr

#define pr_fmt "    "

#define pr_info(fmt, ...)                                      \
    do {                                                       \
        fprintf(logger_out_stream, pr_fmt fmt, ##__VA_ARGS__); \
    } while (0)

#define pr_err(fmt, ...)                                       \
    do {                                                       \
        fprintf(logger_err_stream, pr_fmt fmt, ##__VA_ARGS__); \
    } while (0)

void dump_stack(void)
{
#define STACK_BUF_SIZE 32
    char **stack_info;
    int nr = 0;
    void *buf[STACK_BUF_SIZE];

    nr = backtrace(buf, STACK_BUF_SIZE);
    stack_info = backtrace_symbols(buf, nr);

    pr_err("========== dump stack start ==========\n");
    for (int i = 0; i < nr; i++)
        pr_err("  %s\n", stack_info[i]);
    pr_err("========== dump stack  end  ==========\n");
#undef STACK_BUF_SIZE
}

#define BUG_ON(cond, fmt, ...)                                     \
    do {                                                           \
        if (unlikely(cond)) {                                      \
            pr_err("BUG ON: " #cond ", " fmt "\n", ##__VA_ARGS__); \
            dump_stack();                                          \
            exit(-1);                                              \
        }                                                          \
    } while (0)

#define WARN_ON(cond, fmt, ...)                                    \
    ({                                                             \
        int __w_r_ret = !!(cond);                                  \
        if (unlikely(__w_r_ret))                                   \
            pr_err("WARN ON:" #cond ", " fmt "\n", ##__VA_ARGS__); \
        unlikely(__w_r_ret);                                       \
    })

#define UNIT_BUG_ON(cond)                                 \
    do {                                                  \
        int __u_b_ret;                                    \
        __u_b_ret = (cond);                               \
        printf("[UNIT TEST] %-32s ... ", #cond);          \
        if (!__u_b_ret)                                   \
            printf("\e[32msuccess\e[0m\n");               \
        else                                              \
            printf("\e[31mfailed\e[0m\n");                \
        BUG_ON(__u_b_ret, #cond " failed:%d", __u_b_ret); \
    } while (0)

#endif /* __PRINTUT_H__ */
