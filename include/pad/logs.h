#ifndef __PAD_LOGS_H__
#define __PAD_LOGS_H__

#include <stdio.h>

#ifndef likely
#define likely(x) __builtin_expect(!!(x), 1)
#endif

#ifndef unlikely
#define unlikely(x) __builtin_expect(!!(x), 0)
#endif

#define logger_out_stream stdout
#define logger_err_stream stderr

#define pr_fmt ""

#define pr_info(fmt, ...)                                      \
    do {                                                       \
        fprintf(logger_out_stream, pr_fmt fmt, ##__VA_ARGS__); \
    } while (0)

#define pr_err(fmt, ...)                                       \
    do {                                                       \
        fprintf(logger_err_stream, pr_fmt fmt, ##__VA_ARGS__); \
    } while (0)

#ifdef CONFIG_DEBUG
#define pr_debug pr_info
#else
#define pr_debug(...)
#endif

void pad_dump_stack(void);
void __pad_exit(int exit_code);

#define BUG_ON(cond, fmt, ...)                                     \
    do {                                                           \
        if (unlikely(cond)) {                                      \
            pr_err("BUG ON: " #cond ", " fmt "\n", ##__VA_ARGS__); \
            pad_dump_stack();                                      \
            __pad_exit(-1);                                        \
        }                                                          \
    } while (0)

#define WARN_ON(cond, fmt, ...)                                    \
    ({                                                             \
        int __w_r_ret = !!(cond);                                  \
        if (unlikely(__w_r_ret))                                   \
            pr_err("WARN ON:" #cond ", " fmt "\n", ##__VA_ARGS__); \
        unlikely(__w_r_ret);                                       \
    })

#endif /* __PAD_LOGS_H__*/
