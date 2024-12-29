#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

// Target function to be called
void target_function() {
    printf("Target function called via injected call!\n");
}

// Original function to patch
void __attribute__((patchable_function_entry(16))) __attribute__((section(".probe"))) my_function() {
    printf("Original my_function called.\n");
}

// Get the page size
size_t get_page_size() {
    return (size_t)sysconf(_SC_PAGESIZE);
}

// Align address to the page boundary
void *align_to_page(void *addr, size_t page_size) {
    return (void *)((uintptr_t)addr & ~(page_size - 1));
}

// Patch the function with a call to the target function
void patch_with_call(void *func_address, void *target_address) {
    // Machine code for "call <target_address>" (x86-64)
    unsigned char call_instruction[16];

    // Opcode for call (relative address): 0xE8
    call_instruction[0] = 0xE8;

    // Calculate the relative address from the patched address
    int32_t relative_offset = (int32_t)((uintptr_t)target_address - (uintptr_t)func_address - 5);
    memcpy(call_instruction + 1, &relative_offset, sizeof(relative_offset));

    // Fill the rest with NOPs (optional, for alignment)
    memset(call_instruction + 5, 0x90, sizeof(call_instruction) - 5);

    // Get page size and align to page boundary
    size_t page_size = get_page_size();
    void *page_start = align_to_page(func_address, page_size);

    // Change memory permissions to allow writing
    if (mprotect(page_start, page_size, PROT_READ | PROT_WRITE | PROT_EXEC) < 0) {
        perror("mprotect failed");
        return;
    }

    // Inject the call instruction
    memcpy(func_address, call_instruction, sizeof(call_instruction));

    // Restore memory permissions to read and execute
    if (mprotect(page_start, page_size, PROT_READ | PROT_EXEC) < 0) {
        perror("mprotect restore failed");
        return;
    }

    printf("Patched function at %p to call %p.\n", func_address, target_address);
}

int main() {
    // Get the address of my_function and target_function
    void *func_address = (void *)my_function;
    void *target_address = (void *)target_function;

    printf("Original my_function address: %p\n", func_address);
    printf("Target function address: %p\n", target_address);

    // Call the original function
    printf("Calling my_function normally:\n");
    my_function();

    // Patch my_function with a call to target_function
    patch_with_call(func_address, target_address);

    // Call the patched function
    printf("Calling my_function after patching:\n");
    my_function();

    return 0;
}
