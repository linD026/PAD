#include <uapi/pad.h>
#include <pad/logs.h>

#include <stdio.h>

#include <string.h>
#include <unistd.h>
#include <stddef.h>
#include <stdint.h>

static void decode_code(unsigned char *code)
{
#ifdef CONFIG_DEBUG
    for (int i = 0; i < CONFIG_PAD_PATCHABLE_SPACE; i++) {
        printf("%02x ", code[i]);
    }
    printf("\n");
#endif
}

/* x86-64 */
void arch_init_inject_code(unsigned char *inject_code, uintptr_t target_address,
                           uintptr_t handler)
{
    // Calculate the relative address from the patched address
    int32_t rel;

    BUG_ON(sizeof(ptrdiff_t) >= CONFIG_PAD_PATCHABLE_SPACE,
           "ptrdiff_t (%zu) > CONFIG_PAD_PATCHABLE_SPACE(%d)",
           sizeof(ptrdiff_t), CONFIG_PAD_PATCHABLE_SPACE);

    /*
     *
     * 32-bit address.
     *
     * injected_function:
     *          call <relative_offset>
     *          0xE8        ...
     *   bytes:   1          4         = 5
     */
    inject_code[0] = 0xE8;
    rel = (int32_t)(handler - target_address) - 5;
    memcpy(&inject_code[1], &rel, sizeof(rel));
    memset(&inject_code[5], 0x90, CONFIG_PAD_PATCHABLE_SPACE - 5);

    /*
     * RIP-relative addressing.
     *
     * injected_function:
     *          CALL [rip + offset]
     */
    // TODO: support 64-bit
    BUG_ON(target_address & ~((unsigned long)(1 << 31) - 1),
           "target address(%p) overflow (0x%lx)", (void *)target_address,
           target_address & ~((unsigned long)(1 << 31) - 1));
    BUG_ON(handler & ~((unsigned long)(1 << 31) - 1),
           "handler (%p) overflow (0x%lx)", (void *)handler,
           handler & ~((unsigned long)(1 << 31) - 1));

    decode_code(inject_code);
}

void arch_init_recover_code(unsigned char *inject_code, void *handler)
{
    /* Fill the rest with NOPs (optional, for alignment) */
    memset(inject_code, 0x90, CONFIG_PAD_PATCHABLE_SPACE);
}
