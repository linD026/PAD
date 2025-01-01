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

/* pad_probe->flags */
#define PAD_SET_SHMEM_FLAG 0x0001
#define PAD_EXTERNAL_HANDLER_FLAG 0x0002
#define PAD_FLAGS_MASK (PAD_SET_SHMEM_FLAG | PAD_EXTERNAL_HANDLER_FLAG)

struct pad_probe {
    /* target function address */
    unsigned long address;
    /* tareget fucntion name */
    const char *name;
    unsigned long breakpoint;
    unsigned int flags;
};

#define PAD_ENTER_POINT(name) void name(void)

void pad_builtin_handler(void);
typedef void (*pad_handler_t)(void);

int pad_init(pad_handler_t handler, unsigned int flags);
int pad_exit(void);

int pad_register_probe(struct pad_probe *p);
int pad_unregister_probe(struct pad_probe *p);

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
