#include <pad/logs.h>
#include <pad/shmem.h>

#undef pr_fmt
#define pr_fmt "[shmem] "

struct shmem_data *init_shmem(int pid)
{
    return NULL;
}

void exit_shmem(struct shmem_data *s)
{
}

int ack_shmem(struct shmem_data *s)
{
    return 0;
}

int wait_shmem(struct shmem_data *s)
{
    return 0;
}

int get_data_shmem(struct shmem_data *s)
{
    return 0;
}
