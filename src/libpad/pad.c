#include <uapi/pad.h>

#include <stdatomic.h>
#include <signal.h>

/* pad_probe_data->flags */
#define PP_SHARED_MEM 0x0001
#define PP_CREATED_SHARED_MEM 0x0002
#define PP_INTERNAL_MEM 0x0004

struct pad_probe_data {
    struct pad_probe probe;
    struct list_head node;
    unsigned int flags;
};

sig_atomic_t process_probe_require = 0;

void text_inject(unsigned long address)
{
}

void text_clear(unsigned long address)
{
}

void probe_require_handler(void)
{
}

int pad_init(void)
{
    // check the cflags
    // init mapping
}

int pad_exit(void)
{
    // free the mapping
}

int pad_register_probe(struct pad_probe *p)
{
}

int pad_unregister_probe(struct pad_probe *p)
{
}
