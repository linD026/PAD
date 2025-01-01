/* arch: x86-64 */
#include <uapi/pad.h>
#include <pad/logs.h>

#include <stdio.h>

#include <string.h>
#include <unistd.h>
#include <stddef.h>
#include <stdint.h>

#undef pr_fmt
#define pr_fmt "[x86] "

static void decode_code(unsigned char *code)
{
#ifdef CONFIG_DEBUG
    for (int i = 0; i < CONFIG_PAD_PATCHABLE_SPACE; i++) {
        printf("%02x ", code[i]);
    }
    printf("\n");
#endif
}

void arch_init_inject_code(unsigned char *inject_code, uintptr_t target_address,
                           uintptr_t handler)
{
    // Calculate the relative address from the patched address
    ptrdiff_t rel;

    /*
     *
     * 32-bit address.
     *
     * injected_function:
     *          call <relative_offset>
     *          0xE8        ...
     *   bytes:   1          4         = 5
     *
     * E8: Call near, relative, displacement relative to next instruction.
     *     32-bit displacement sign extended to 64-bits in 64-bit mode.
     */
    //    inject_code[0] = 0xE8;
    //    rel = (ptrdiff_t)(handler - target_address) - 5;
    //    memcpy(&inject_code[1], &rel, sizeof(rel));
    //    memset(&inject_code[5], 0x90, CONFIG_PAD_PATCHABLE_SPACE - 5);

    /*
     * 64-bit absolute address.
     *
     * We do:
     *      movabs <address>, %rax
     *      call   *rax
     *      nop
     */
    inject_code[0] = 0x48; // REX prefix (64-bit operand size)
    inject_code[1] = 0xB8; // 'mov' instruction opcode

    for (int i = 0; i < 8; i++) {
        inject_code[i + 2] = (handler >> (i * 8)) & 0xFF;
    }

    inject_code[10] = 0xFF; // 'call rax' instruction
    inject_code[11] = 0xD0; // ModRM byte for 'call rax'

    memset(&inject_code[12], 0x90, CONFIG_PAD_PATCHABLE_SPACE - 12);

    decode_code(inject_code);
}

void arch_init_recover_code(unsigned char *inject_code, void *handler)
{
    /* Fill the rest with NOPs (optional, for alignment) */
    memset(inject_code, 0x90, CONFIG_PAD_PATCHABLE_SPACE);
}
