#ifndef __UAPI_PAD_H__
#define __UAPI_PAD_H__

#ifndef CONFIG_PAD_PATCHABLE_SPACE
#define CONFIG_PAD_PATCHABLE_SPACE 16
#endif

#ifndef CONFIG_PAD_SECTION
#define CONFIG_PAD_SECTION ".pad_probe"
#endif

#define __pad_trace                                                       \
    __attribute__((aligned(64))) __attribute__((optimize(0)))             \
    __attribute__((patchable_function_entry(CONFIG_PAD_PATCHABLE_SPACE))) \
    __attribute__((section(CONFIG_PAD_SECTION)))

#define __pad_handler                                         \
    __attribute__((aligned(64))) __attribute__((optimize(0))) \
    __attribute__((section(CONFIG_PAD_SECTION)))

#define PAD_ENTER_POINT(name) void name(void)

struct pad_probe {
    unsigned long address;
    const char *name;
};

void __probe_handler(void);

int pad_init(void);
int pad_exit(void);

/* debug testing */
#ifdef CONFIG_DEBUG
void pad_test_inject(unsigned long address, unsigned long function);
void pad_test_recover(unsigned long address);
#else
static inline void pad_test_inject(unsigned long address,
                                   unsigned long function)
{
}
static inline void pad_test_recover(unsigned long address)
{
}
#endif /* CONFIG_DEBUG */

#endif /* __UAPI_PAD_H__ */
