#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>
#include <string.h>
#include <unistd.h>

#include "../../include/uapi/pad.h"

void __pad_trace target_func()
{
    printf("Original target_func executed.\n");
}

void handler_func()
{
    printf("Handler function executed.\n");
}

void patch_function_to_inline_call(void *target_func, void *handler_func)
{
    // Ensure proper memory permissions
    size_t page_size = sysconf(_SC_PAGESIZE);
    uintptr_t page_start = (uintptr_t)target_func & ~(page_size - 1);

    if (mprotect((void *)page_start, page_size,
                 PROT_READ | PROT_WRITE | PROT_EXEC) < 0) {
        perror("mprotect failed");
        return;
    }

    // Assembly instructions to inject:
    // call [rip + 0x00]   (7 bytes)
    // handler_func_address (8 bytes)

    uint8_t code[15];

    // call [rip + 0x00]
    code[0] = 0xFF; // Opcode for indirect call
    code[1] = 0x15; // ModR/M for RIP-relative addressing
    int32_t relative_offset =
        7; // Offset from the next instruction to the address
    memcpy(&code[2], &relative_offset, sizeof(relative_offset));

    // Inline the handler address
    memcpy(&code[6], &handler_func, sizeof(handler_func));

    // Inject the code into the target function
    memcpy(target_func, code, sizeof(code));

    // Restore memory permissions
    if (mprotect((void *)page_start, page_size, PROT_READ | PROT_EXEC) < 0) {
        perror("mprotect restore failed");
    }

    printf("Patched function at %p to call handler at %p\n", target_func,
           handler_func);
}

int main()
{
    printf("Before patching:\n");
    target_func();

    // Patch the target function to call the handler
    patch_function_to_inline_call((void *)target_func, (void *)handler_func);

    printf("After patching:\n");
    target_func();

    return 0;
}
