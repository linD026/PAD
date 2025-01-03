#define _GNU_SOURCE /* for rwlock, _SC_PAGESIZE */
#include <arch/common.h>
#include <uapi/pad.h>
#include <pad/logs.h>
#include <pad/shmem.h>
#include <pad/list.h>

#include <errno.h>
#include <pthread.h>
#include <stdatomic.h>
#include <signal.h>
#include <dlfcn.h>
#include <string.h> /* for memcpy */
#include <stdlib.h> /* for malloc() */
#include <sys/mman.h> /* for page_size */
#include <assert.h> /* for static_assert() */

#undef pr_fmt
#define pr_fmt "[libpad] "

/*
 * TODO: For future debugging feature
 */
#define PP_SHARED_MEM 0x0001
#define PP_CREATED_SHARED_MEM 0x0002
#define PP_INTERNAL_MEM 0x0004

struct breakpoint_info {
    unsigned long breakpoint;
    unsigned int flags;
    struct list_head bp_node;
};

struct target {
    unsigned long address;
    struct list_head bp_head;
    pthread_rwlock_t lock;

    /* Protected by pad_data->lock. */
    int refcount;
    struct list_head node;
};

struct pad_data {
    struct list_head list;
    pthread_mutex_t lock;
};

static struct pad_data pad_data;

/* x86-64 instruction is 12 bytes. */
static_assert(CONFIG_PAD_PATCHABLE_SPACE > 12,
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
    return (void *)((unsigned long)addr & ~(page_size - 1));
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

static void arm_pad(struct target *target)
{
    pad_handler_t handler = NULL;

    handler = atomic_load_explicit(&pad_arm_handler, memory_order_acquire);

    text_inject(target->address, (unsigned long)handler);
}

static void disarm_pad(struct target *target)
{
    text_clear(target->address);
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

    // register

    atomic_fetch_add_explicit(&process_probe_require, -1, memory_order_release);
}

/* Return the previous refcount. */
static int insert_breakpoint(struct target *target, struct pad_probe *p)
{
    struct breakpoint_info *prealloc = NULL;
    struct breakpoint_info *bi = NULL;
    int ret = -ENOMEM;

    prealloc = malloc(sizeof(struct breakpoint_info));
    if (unlikely(!prealloc))
        return ret;

    prealloc->breakpoint = p->breakpoint;
    prealloc->flags = p->flags;
    list_init(&prealloc->bp_node);

    ret = -EINVAL;

    pthread_rwlock_wrlock(&target->lock);

    list_for_each_entry (bi, &target->bp_head, bp_node) {
        if (bi->breakpoint == p->breakpoint) {
            pthread_rwlock_unlock(&target->lock);
            ret = 0;
            goto out;
        }
    }

    bi = prealloc;
    prealloc = NULL;
    list_add_tail(&bi->bp_node, &target->bp_head);
    ret = 0;

    pthread_rwlock_unlock(&target->lock);

#ifdef CONFIG_DEBUG
    pr_info("inserted\n");
#endif

out:
    if (prealloc)
        free(prealloc);

    return ret;
}

static int delete_breakpoint(struct target *target, struct pad_probe *p)
{
    struct breakpoint_info *bi = NULL;
    int ret = -EINVAL;

    pthread_rwlock_wrlock(&target->lock);

    list_for_each_entry (bi, &target->bp_head, bp_node) {
        if (bi->breakpoint == p->breakpoint) {
            list_del(&bi->bp_node);
            ret = 0;
            break;
        }
    }

    pthread_rwlock_unlock(&target->lock);

    return ret;
}

/* User APIs. */

void __attribute__((aligned(64))) __attribute__((optimize(0)))
pad_builtin_handler(void)
{
    struct target *target = NULL;
    struct breakpoint_info *bi = NULL;
    unsigned long address = 0;

    address = arch_get_target_address();
    if (unlikely(!address)) {
        WARN_ON(1, "unsupport arch_get_target_address()");
        return;
    }

#ifdef CONFIG_DEBUG
    pr_info("address:%p\n", (void *)address);
#endif

    pthread_mutex_lock(&pad_data.lock);
    list_for_each_entry (target, &pad_data.list, node) {
        if (target->address == address) {
            /* Prevent someone release this while we are handling. */
            target->refcount++;
            pthread_mutex_unlock(&pad_data.lock);

            // TODO: Don't call the handler when holding rlock.
            pthread_rwlock_rdlock(&target->lock);
            list_for_each_entry (bi, &target->bp_head, bp_node) {
                pad_handler_t handler = (pad_handler_t)bi->breakpoint;
                handler();
            }
            pthread_rwlock_unlock(&target->lock);

            pthread_mutex_lock(&pad_data.lock);
            target->refcount--;
            pthread_mutex_unlock(&pad_data.lock);

            return;
        }
    }
    pthread_mutex_unlock(&pad_data.lock);
}

int pad_init(pad_handler_t handler, unsigned int flags)
{
    int ret = 0;

    pad_flags = flags & PAD_FLAGS_MASK;

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

    atomic_store_explicit(&pad_arm_handler, handler, memory_order_release);

    pthread_mutex_init(&pad_data.lock, NULL);
    list_init(&pad_data.list);

bail_out:

    return ret;
}

int pad_exit(void)
{
    if (shmem_data)
        exit_shmem(shmem_data);

    //TODO: delete the list

    return 0;
}

int pad_register_probe(struct pad_probe *p)
{
    struct target *prealloc = NULL;
    struct target *target = NULL;
    int ret = -EINVAL;

    if (unlikely(!p))
        return ret;

    if (unlikely(!p->breakpoint))
        return ret;

    if (!p->address && p->name) {
        p->address = (unsigned long)dlsym(RTLD_DEFAULT, p->name);
        if (!p->address)
            return ret;
    }

    ret = -ENOMEM;
    prealloc = malloc(sizeof(struct target));
    if (unlikely(!prealloc))
        return ret;

    prealloc->address = p->address;
    list_init(&prealloc->bp_head);
    prealloc->refcount = 0;
    pthread_rwlock_init(&prealloc->lock, NULL);
    list_init(&prealloc->node);

    pthread_mutex_lock(&pad_data.lock);

    list_for_each_entry (target, &pad_data.list, node) {
        if (target->address == p->address) {
            pthread_mutex_unlock(&pad_data.lock);
            ret = insert_breakpoint(target, p);
            goto out;
        }
    }

    target = prealloc;
    prealloc = NULL;
    pthread_mutex_unlock(&pad_data.lock);

    ret = insert_breakpoint(target, p);

out:
    if (prealloc)
        free(prealloc);

    if (likely(!ret)) {
        pthread_mutex_lock(&pad_data.lock);
        ret = target->refcount++;
        list_add_tail(&target->node, &pad_data.list);
        pthread_mutex_unlock(&pad_data.lock);
        if (!ret)
            arm_pad(target);
    }

    return ret;
}

int pad_unregister_probe(struct pad_probe *p)
{
    struct target *target = NULL;
    int ret = -EINVAL;

    if (unlikely(!p))
        return ret;

    if (unlikely(!p->breakpoint))
        return ret;

    if (!p->address && p->name) {
        p->address = (unsigned long)dlsym(RTLD_DEFAULT, p->name);
        if (!p->address)
            return ret;
    }

    pthread_mutex_lock(&pad_data.lock);

    list_for_each_entry (target, &pad_data.list, node) {
        if (target->address == p->address) {
            pthread_mutex_unlock(&pad_data.lock);
            ret = delete_breakpoint(target, p);
            goto out;
        }
    }

    pthread_mutex_unlock(&pad_data.lock);

out:
    if (likely(!ret)) {
        /*
         * we want to delete the node in pad_data->list,
         * it needs to require the lock again.
         */
        pthread_mutex_lock(&pad_data.lock);
        /* Someone might insert new breakpoint, lets check again. */
        if (--target->refcount == 0) {
            /* we can safely delete the node. */
            list_del(&target->node);
            pthread_rwlock_destroy(&target->lock);

            pthread_mutex_unlock(&pad_data.lock);

            disarm_pad(target);

            WARN_ON(!list_empty(&target->bp_head),
                    "delete target while holding breakpoint");
            return ret;
        }
        pthread_mutex_unlock(&pad_data.lock);
    }

    return (ret > 0) ? 0 : ret;
}

/* Debug testing */
#ifdef CONFIG_DEBUG
void pad_test_inject(unsigned long address, unsigned long function)
{
    if (!function)
        function = (unsigned long)pad_builtin_handler;

    text_inject(address + arch_skip_instruction, function);
}

void pad_test_recover(unsigned long address)
{
    text_clear(address + 4);
}
#endif /* CONFIG_DEBUG */
