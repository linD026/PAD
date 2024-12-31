#define _GNU_SOURCE
#include <arch/common.h>
#include <uapi/pad.h>
#include <pad/logs.h>

#include <pthread.h>
#include <stdatomic.h>
#include <signal.h>
#include <string.h>
#include <stdint.h>
#include <sys/mman.h>
#include <unistd.h>
#include <assert.h>

/* pad_probe_data->flags */
#define PP_SHARED_MEM 0x0001
#define PP_CREATED_SHARED_MEM 0x0002
#define PP_INTERNAL_MEM 0x0004

struct pad_probe_data {
    struct pad_probe probe;
    unsigned int flags;
};

static_assert(CONFIG_PAD_PATCHABLE_SPACE > 5,
              "CONFIG_PAD_PATCHABLE_SPACE is too small");

static size_t page_size = 4096;
static sig_atomic_t process_probe_require = 0;
static pthread_mutex_t probe_poke_lock = PTHREAD_MUTEX_INITIALIZER;

static __always_inline void poke_lock(void)
{
    pthread_mutex_lock(&probe_poke_lock);
}

static __always_inline void poke_unlock(void)
{
    pthread_mutex_unlock(&probe_poke_lock);
}

static __always_inline void *align_to_page(void *addr)
{
    return (void *)((uintptr_t)addr & ~(page_size - 1));
}

static void text_permission_start(unsigned long address)
{
    void *page_start = align_to_page((void *)address);
    int ret =
        mprotect(page_start, page_size, PROT_READ | PROT_WRITE | PROT_EXEC);

    WARN_ON(ret < 0, "mprotect error:%d", ret);
}

static void text_permission_end(unsigned long address)
{
    void *page_start = align_to_page((void *)address);
    int ret = mprotect(page_start, page_size, PROT_READ | PROT_EXEC);

    WARN_ON(ret < 0, "mprotect error:%d", ret);
}

static void text_poke(unsigned long address, unsigned char *code)
{
    text_permission_start(address);
    poke_lock();
    memcpy((void *)address, code, CONFIG_PAD_PATCHABLE_SPACE);
    poke_unlock();
    text_permission_end(address);
}

static void text_inject(unsigned long address, unsigned long handler)
{
    unsigned char
        __attribute__((aligned(16))) code[CONFIG_PAD_PATCHABLE_SPACE] = { 0 };

    arch_init_inject_code(code, (void *)address, (void *)handler);

    text_poke(address, code);
}

static void text_clear(unsigned long address)
{
    unsigned char
        __attribute__((aligned(16))) code[CONFIG_PAD_PATCHABLE_SPACE] = { 0 };

    arch_init_recover_code(code, (void *)address);

    text_poke(address, code);
}

void __attribute__((aligned(64))) __attribute__((optimize(0)))
__probe_handler(void)
{
    printf("This is probe function!\n");
}

void probe_require_handler(void)
{
    __probe_handler();
}

int pad_init(void)
{
    // check the cflags
    // init mapping
    page_size = (size_t)sysconf(_SC_PAGESIZE);

    return 0;
}

int pad_exit(void)
{
    // free the mapping
    //dlclose(dlp);
    return 0;
}

int pad_register_probe(struct pad_probe *p)
{
    return 0;
}

int pad_unregister_probe(struct pad_probe *p)
{
    return 0;
}

/* Debug testing */

#ifdef CONFIG_DEBUG
void pad_test_inject(unsigned long address, unsigned long function)
{
    if (!function) {
        // FIXME: 64-bit and ASLR
        function = (unsigned long)__probe_handler;
        BUG_ON(1, "Doesn't support builtin handler");
    }

    // FIXME: dynamically skipping endbr64
    text_inject(address + 4, function);
}

void pad_test_recover(unsigned long address)
{
    text_clear(address + 4);
}
#endif /* CONFIG_DEBUG */
