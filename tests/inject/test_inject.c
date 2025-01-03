#include <stdio.h>

#define __PROTO(args...) args
#define __ARGS(args...) args

#define BREAKPOINTABLE(type, name, proto, args)           \
    type __breakpointable_##name(proto);                  \
    static unsigned long breakpoint_##name##_key = 0x00;  \
    type name(proto)                                      \
    {                                                     \
        if (static_key_enable(breakpoint_##name##_key)) { \
            breakpoint(args);                             \
        }                                                 \
        __breakpointable_##name(args);                    \
    }                                                     \
    type __breakpointable_##name(proto)

int static_key_enable(unsigned long idx)
{
    return idx;
}

void breakpoint(int a)
{
    printf("breakpoint\n");
}

BREAKPOINTABLE(void, breakpoint_func, int a, a)
{
    printf("function\n");
}

int lib_case(void)
{
    breakpoint_func(0);

    breakpoint_breakpoint_func_key = 1;

    breakpoint_func(1);

    breakpoint_breakpoint_func_key = 0;

    breakpoint_func(0);

    return 0;
}

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

unsigned long return_address = 0;

// Original function with patchable entry
void __attribute__((patchable_function_entry(16))) my_function()
{
    printf("Original my_function called.\n");
}

// Replacement function
void replacement_function()
{
    printf("Replacement function called.\n");

    __asm__ volatile("jmp *%0" : : "r"(return_address));
}

// Function to modify the entry to redirect to the replacement function
void modify_function_entry(void *func_address, void *replacement_address)
{
    unsigned char jump_code[16]; // Machine code for jump instruction

    // Construct a relative jump (x86-64: E9 <offset>)
    jump_code[0] = 0xE9; // Opcode for JMP rel32
    int32_t offset =
        (int32_t)((uintptr_t)replacement_address - (uintptr_t)func_address - 5);
    memcpy(jump_code + 1, &offset, sizeof(offset));

    // Fill the rest with NOPs (optional for alignment)
    memset(jump_code + 5, 0x90, 11);

    // Patch the function entry
    memcpy(func_address, jump_code, sizeof(jump_code));

    // TODO;
    return_address = (unsigned long)(func_address - offset);

    printf("Patched function entry at %p to jump to %p.\n", func_address,
           replacement_address);
}

size_t get_page_size()
{
    return (size_t)sysconf(_SC_PAGESIZE);
}

// Function to align an address to the start of its page
void *align_to_page(void *addr, size_t page_size)
{
    return (void *)((uintptr_t)addr & ~(page_size - 1));
}

int main()
{
    printf("Calling my_function normally:\n");
    my_function();

    size_t page_size = get_page_size();
    void *page_start = align_to_page(my_function, page_size);

    printf("Function address: %p\n", my_function);
    printf("Page start address: %p\n", page_start);

    // Change memory permissions to allow writing
    if (mprotect(page_start, page_size, PROT_READ | PROT_WRITE | PROT_EXEC) <
        0) {
        perror("mprotect failed");
        return EXIT_FAILURE;
    }

    // Patch the function to redirect to replacement_function
    modify_function_entry((void *)my_function, (void *)replacement_function);

    printf("Calling my_function after patching:\n");
    my_function(); // This will now call replacement_function

    return 0;
}
