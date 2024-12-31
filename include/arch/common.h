#ifndef __PAD_ARCH_COMMON_H__
#define __PAD_ARCH_COMMON_H__

void arch_init_inject_code(unsigned char *inject_code, void *target_address,
                           void *handler);
void arch_init_recover_code(unsigned char *inject_code, void *handler);

#endif /* __PAD_ARCH_COMMON_H__ */
