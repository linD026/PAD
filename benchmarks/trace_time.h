#ifndef __TRACE_TIME_H__
#define __TRACE_TIME_H__

#include <stdio.h>
#include <time.h>

/* Return the duration of sec + nsec as ns unit */
static inline unsigned long long full_time_diff(struct timespec start,
                                                struct timespec end)
{
    return (1000000000ULL * end.tv_sec + end.tv_nsec) -
           (1000000000ULL * start.tv_sec + start.tv_nsec);
}

#define DECLARE_TIME(name) \
    struct timespec time_##name##_start, time_##name##_end

#define time_get_start(name) \
    clock_gettime(CLOCK_MONOTONIC, &time_##name##_start)
#define time_get_end(name) clock_gettime(CLOCK_MONOTONIC, &time_##name##_end)

#define print_time_ms(name)                                         \
    do {                                                            \
        unsigned long long during =                                 \
            full_time_diff(time_##name##_start, time_##name##_end); \
        during = during / 1000000;                                  \
        printf("%llu\n", during);                                   \
    } while (0)

#define time_ms(name)                                               \
    ({                                                              \
        unsigned long long during =                                 \
            full_time_diff(time_##name##_start, time_##name##_end); \
        during = during / 1000000;                                  \
        during;                                                     \
    })

#define print_time_us(name)                                         \
    do {                                                            \
        unsigned long long during =                                 \
            full_time_diff(time_##name##_start, time_##name##_end); \
        during = during / 1000;                                     \
        printf("%llu\n", during);                                   \
    } while (0)

#define time_us(name)                                               \
    ({                                                              \
        unsigned long long during =                                 \
            full_time_diff(time_##name##_start, time_##name##_end); \
        during = during / 1000;                                     \
        during;                                                     \
    })

#define print_time_ns(name)                                         \
    do {                                                            \
        unsigned long long during =                                 \
            full_time_diff(time_##name##_start, time_##name##_end); \
        printf("%llu\n", during);                                   \
    } while (0)

#define time_ns(name)                                               \
    ({                                                              \
        unsigned long long during =                                 \
            full_time_diff(time_##name##_start, time_##name##_end); \
        during;                                                     \
    })

#endif /* __TRACE_TIME_H__ */
