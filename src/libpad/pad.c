#define _GNU_SOURCE
#include <arch/common.h>
#include <uapi/pad.h>
#include <pad/logs.h>
#include <pad/shmem.h>

#include <errno.h>
#include <pthread.h>
#include <stdatomic.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>
#include <stdint.h> /* for uintptr_t */
#include <sys/mman.h> /* for page_size */
#include <assert.h> /* for static_assert() */
#include <stddef.h> /* for ptrdiff_t */

#undef pr_fmt
#define pr_fmt "[libpad] "

/*
 * TODO: For future debugging feature
 */
#define PP_SHARED_MEM 0x0001
#define PP_CREATED_SHARED_MEM 0x0002
#define PP_INTERNAL_MEM 0x0004

/* TODO: Per-probe handler. */
struct pad_probe_data {
    struct pad_probe *p;
    /* list */
};

/* x86-64 instruction is 5 bytes. */
static_assert(CONFIG_PAD_PATCHABLE_SPACE > 5,
              "CONFIG_PAD_PATCHABLE_SPACE is too small");
static_assert(sizeof(ptrdiff_t) < CONFIG_PAD_PATCHABLE_SPACE,
              "ptrdiff_t is too small");
static_assert(sizeof(ptrdiff_t) >= 4, "ptrdiff_t is smaller than 32 bits");

static size_t page_size = 4096;
static pthread_mutex_t probe_poke_lock = PTHREAD_MUTEX_INITIALIZER;
static _Atomic pad_handler_t pad_arm_handler;
static struct shmem_data *shmem_data = NULL;
static unsigned int pad_flags = 0;

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
    /* Prefer not to call the mprotect while holding the lock. */
    poke_lock();
    memcpy((void *)address, code, CONFIG_PAD_PATCHABLE_SPACE);
    poke_unlock();
    /* Ditto, see above comments. */
    text_permission_end(address);
}

static void text_inject(unsigned long address, unsigned long handler)
{
    unsigned char __attribute__((aligned(
        CONFIG_PAD_PATCHABLE_SPACE))) code[CONFIG_PAD_PATCHABLE_SPACE] = { 0 };

    arch_init_inject_code(code, (void *)address, (void *)handler);

    text_poke(address, code);
}

static void text_clear(unsigned long address)
{
    unsigned char __attribute__((aligned(
        CONFIG_PAD_PATCHABLE_SPACE))) code[CONFIG_PAD_PATCHABLE_SPACE] = { 0 };

    arch_init_recover_code(code, (void *)address);

    text_poke(address, code);
}

// TODO: we might insert multiple function to the same spot.
// TODO: insert to the list instead of inject the function
// TODO: per-function critical section
static void arm_pad(struct pad_probe *p)
{
    pad_handler_t handler = NULL;

    atomic_signal_fence(memory_order_acquire);
    handler = atomic_load_explicit(&pad_arm_handler, memory_order_relaxed);

    text_inject(p->address, (unsigned long)handler);
}

static void disarm_pad(struct pad_probe *p)
{
    text_clear(p->address);
}

/*
 * signal handler
 *
 * The rquest from signal doesn't have a pad_probe_data, so lets check the
 * pad_flags.
 */
static atomic_int process_probe_require = 0;

static void probe_require_handler(int signal)
{
    struct pad_probe p = { 0 };
    unsigned long target_address = 0;

    /*
     * TODO: Check there doesn't have multiple signal raised at the same time.
     */
    while (atomic_load_explicit(&process_probe_require, memory_order_acquire))
        ;

    atomic_fetch_add_explicit(&process_probe_require, 1, memory_order_release);

    if (pad_flags & PAD_SET_SHMEM_FLAG && unlikely(!shmem_data)) {
        shmem_data = init_shmem(getpid());
        if (unlikely(!shmem_data)) {
            WARN_ON(1, "in signal, failed to set the shmem data");
            return;
        }
    }

    target_address = get_data_shmem(shmem_data);

    p.address = target_address;

    arm_pad(&p);

    atomic_fetch_add_explicit(&process_probe_require, -1, memory_order_release);
}

/* User APIs. */

void __attribute__((aligned(64))) __attribute__((optimize(0)))
pad_builtin_handler(void)
{
    // TODO: call the list of functions
    printf("This is probe function!\n");
}

int pad_init(pad_handler_t handler, unsigned int flags)
{
    int ret = 0;

    pad_flags = flags & PAD_FLAGS_MASK;

    // TODO: impl the internal (builtin) handler feature (64-bit addr problem)
    pad_flags |= PAD_EXTERNAL_HANDLER_FLAG;

    if (pad_flags & PAD_EXTERNAL_HANDLER_FLAG) {
        if (unlikely(!handler)) {
            ret = -EINVAL;
            goto bail_out;
        }
    } else {
        handler = pad_builtin_handler;
    }

    if (pad_flags & PAD_SET_SHMEM_FLAG) {
        shmem_data = init_shmem(getpid());
        if (unlikely(!shmem_data)) {
            ret = -EINVAL;
            goto bail_out;
        }
    }

    page_size = (size_t)sysconf(_SC_PAGESIZE);

    signal(SIGTRAP, probe_require_handler);

    atomic_store_explicit(&pad_arm_handler, handler, memory_order_relaxed);
    atomic_signal_fence(memory_order_release);

bail_out:

    return ret;
}

int pad_exit(void)
{
    if (shmem_data)
        exit_shmem(shmem_data);

    return 0;
}

int pad_register_probe(struct pad_probe *p)
{
    if (unlikely(!p))
        return -EINVAL;

    if (!p->address && p->name) {
        p->address = (unsigned long)dlsym(RTLD_DEFAULT, p->name);
        if (!p->address)
            return -EINVAL;
    }

    arm_pad(p);

    return 0;
}

int pad_unregister_probe(struct pad_probe *p)
{
    disarm_pad(p);
    return 0;
}

/* Debug testing */
#ifdef CONFIG_DEBUG
void pad_test_inject(unsigned long address, unsigned long function)
{
    if (!function) {
        // FIXME: 64-bit and ASLR
        function = (unsigned long)pad_builtin_handler;
        //BUG_ON(1, "Doesn't support builtin handler");
    }

    // FIXME: dynamically skipping endbr64
    text_inject(address + 4, function);
}

void pad_test_recover(unsigned long address)
{
    text_clear(address + 4);
}
#endif /* CONFIG_DEBUG */
