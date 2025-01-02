#ifndef __PAD_ARCH_COMMON_H__
#define __PAD_ARCH_COMMON_H__

void arch_init_inject_code(unsigned char *inject_code, void *target_address,
                           void *handler);
void arch_init_recover_code(unsigned char *inject_code, void *handler);

#ifdef CONFIG_ARCH_X86_64
#define arch_get_target_address()                                     \
    ({                                                                \
        void *__arch_c_a;                                             \
                                                                      \
        __asm__ volatile("movq 8(%%rbp), %0" : "=r"(__arch_c_a) : :); \
                                                                      \
        (unsigned long)__arch_c_a - 12;                               \
    })
#else
#define arch_get_target_address() ((unsigned long)0)
#endif

extern unsigned long arch_skip_instruction;

#endif /* __PAD_ARCH_COMMON_H__ */
