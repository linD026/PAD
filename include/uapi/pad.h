#ifndef __UAPI_PAD_H__
#define __UAPI_PAD_H__

#ifndef CONFIG_PAD_PATCHABLE_SPACE
#define CONFIG_PAD_PATCHABLE_SPACE 16
#endif

#ifndef CONFIG_PAD_SECTION
#define CONFIG_PAD_SECTION ".pad_probe"
#endif

#define __pad_trace                                                       \
    __attribute__((patchable_function_entry(CONFIG_PAD_PATCHABLE_SPACE))) \
    __attribute__((section(CONFIG_PAD_SECTION)))

#define PAD_ENTER_POINT(name) void name(void)

struct pad_probe {
    unsigned long address;
    const char *name;
};

int pad_init(void);
int pad_exit(void);

#endif /* __UAPI_PAD_H__ */
