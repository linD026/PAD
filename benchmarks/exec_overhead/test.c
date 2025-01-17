#define _POSIX_C_SOURCE 199309L
#include "../trace_time.h"
#include "../../include/uapi/pad.h"

int __pad_trace tracee(int a, int b)
{
    return a + b;
}

int normal(int a, int b)
{
    return a + b;
}

PAD_ENTER_POINT(breakpoint)
{
    return;
}

#define NR_TIME 10000

int main(void)
{
    DECLARE_TIME(record_normal);
    DECLARE_TIME(record_traced);
    DECLARE_TIME(record_traced_inserted);

    struct pad_probe p = {
        .address = (unsigned long)tracee,
        .breakpoint = (unsigned long) breakpoint,
    };
    pad_init(0, 0);

    time_get_start(record_normal);
    for (int i = 0; i < NR_TIME; i++) {
        normal(i, i + 1);
    }
    time_get_end(record_normal);
    
    time_get_start(record_traced);
    for (int i = 0; i < NR_TIME; i++) {
        tracee(i, i + 1);
    }
    time_get_end(record_traced);
   
    pad_register_probe(&p);

    time_get_start(record_traced_inserted);
    for (int i = 0; i < NR_TIME; i++) {
        tracee(i, i + 1);
    }
    time_get_end(record_traced_inserted);


    pad_unregister_probe(&p);
    pad_exit();


    printf("%llu %llu %llu\n",
            time_ns(record_normal) / NR_TIME,
            time_ns(record_traced) / NR_TIME,
            time_ns(record_traced_inserted) / NR_TIME);
}
