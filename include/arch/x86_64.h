#ifndef __PAD_ARCH_X86_64_H__
#define __PAD_ARCH_X86_64_H__

#ifndef arch_get_target_address
#define arch_get_target_address()                                     \
    ({                                                                \
        void *__arch_c_a;                                             \
                                                                      \
        __asm__ volatile("movq 8(%%rbp), %0" : "=r"(__arch_c_a) : :); \
                                                                      \
        (unsigned long)__arch_c_a - 12;                               \
    })
#endif

#endif /* __PAD_ARCH_X86_64_H__ */
