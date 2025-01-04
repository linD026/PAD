// TODO: Fix this
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200112L
#endif

#include <pad/logs.h>
#include <pad/shmem.h>

#include <sys/mman.h> /* for shm_open */
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h> /* for malloc */
#include <errno.h>
#include <unistd.h> /* for ftruncate */
#include <string.h> /* for memset */

#undef pr_fmt
#define pr_fmt "[shmem] "

struct shmem_data *init_shmem(int pid)
{
    struct shmem_data *s;

    s = malloc(sizeof(struct shmem_data));
    if (WARN_ON(!s, "shmem malloc"))
        return NULL;

    s->pid = pid;

    snprintf(s->path_name, FIXED_BUF_SIZE, CONFIG_SHMEM_ROOT "/%d", pid);
    s->path_name[FIXED_BUF_SIZE - 1] = '\0';

    s->fd = shm_open(s->path_name, O_CREAT | O_RDWR, 0600);
    if (WARN_ON(s->fd == -1, "shm_open(%s)", s->path_name))
        goto cleanup_alloc;
    // TODO: already existed?

    if (unlikely(ftruncate(s->fd, sizeof(struct shared_shmem_data)) == -1))
        goto cleanup_open;

    s->shared = mmap(NULL, sizeof(struct shared_shmem_data),
                     PROT_READ | PROT_WRITE, MAP_SHARED, s->fd, 0);
    if (WARN_ON(s->shared == MAP_FAILED, "shmem mmap"))
        goto cleanup_open;

    if (WARN_ON(sem_init(&s->shared->ack, 1, 0) == -1, "shmem sem_init"))
        goto cleanup_mmap;

    memset(s->shared->symbol, '\0', CONFIG_SHMEM_BUF_SIZE);
    memset(s->shared->path, '\0', CONFIG_SHMEM_BUF_SIZE);

    return s;

cleanup_mmap:
    munmap(s->shared, sizeof(struct shared_shmem_data));
cleanup_open:
    shm_unlink(s->path_name);
cleanup_alloc:
    free(s);

    return NULL;
}

void exit_shmem(struct shmem_data *s)
{
    if (WARN_ON(!s, "shmem_data is NULL"))
        return;
    munmap(s->shared, sizeof(struct shared_shmem_data));
    shm_unlink(s->path_name);
    free(s);
}

int ack_shmem(struct shmem_data *s)
{
    return sem_post(&s->shared->ack);
}

int wait_shmem(struct shmem_data *s)
{
    return sem_wait(&s->shared->ack);
}

int post_data_shmem(char *restrict shared_buffer, char *restrict data)
{
    memcpy(shared_buffer, data, FIXED_BUF_SIZE);
    return 0;
}

int get_data_shmem(char *restrict buffer, const char *restrict shared_buffer)
{
    memcpy(buffer, shared_buffer, FIXED_BUF_SIZE);
    return 0;
}
