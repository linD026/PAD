#ifndef __COMMON_H__
#define __COMMON_H__

#define COHERENCE_SIZE 128
#define ___aligned__ __attribute__((aligned(COHERENCE_SIZE)))

#define cmb() __asm__ __volatile__("" : : : "memory")

#ifndef likely
#define likely(x) __builtin_expect(!!(x), 1)
#endif

#ifndef unlikely
#define unlikely(x) __builtin_expect(!!(x), 0)
#endif

#ifndef container_of
#define container_of(ptr, type, member)                        \
    __extension__({                                            \
        const __typeof__(((type *)0)->member) *__mptr = (ptr); \
        (type *)((char *)__mptr - offsetof(type, member));     \
    })
#endif

#ifndef __always_inline
#define __always_inline inline __attribute__((__always_inline__))
#endif

#ifndef __noinline
#define __noinline __attribute__((__noinline__))
#endif

#ifndef __allow_unused
#define __allow_unused __attribute__((unused))
#endif

#ifndef macro_var_args_count
#define macro_var_args_count(...) \
    (sizeof((void *[]){ 0, __VA_ARGS__ }) / sizeof(void *) - 1)
#endif

#ifndef ___PASTE
#define ___PASTE(a, b) a##b
#endif

#ifndef __PASTE
#define __PASTE(a, b) ___PASTE(a, b)
#endif

#ifndef __UNIQUE_ID
#define __UNIQUE_ID(prefix) __PASTE(__PASTE(__UNIQUE_ID_, prefix), __LINE__)
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

#ifndef min
#define min(l, r) ((l < r) ? l : r)
#endif

#ifndef max
#define max(l, r) ((l > r) ? l : r)
#endif

#endif /* __COMMON_H__ */
