#ifndef __PAD_SHMEM_H__
#define __PAD_SHMEM_H__

#include <semaphore.h>

#ifndef FIXED_BUF_SIZE
#define FIXED_BUF_SIZE 256
#endif

#ifndef CONFIG_SHMEM_ROOT
#define CONFIG_SHMEM_ROOT ""
#endif

#ifndef CONFIG_SHMEM_BUF_SIZE
#define CONFIG_SHMEM_BUF_SIZE (FIXED_BUF_SIZE + FIXED_BUF_SIZE)
#endif

struct shared_shmem_data {
    sem_t ack;
    char symbol[CONFIG_SHMEM_BUF_SIZE];
    char path[CONFIG_SHMEM_BUF_SIZE];
};

struct shmem_data {
    int fd;
    int pid;
    char path_name[FIXED_BUF_SIZE];
    struct shared_shmem_data *shared;
};

struct shmem_data *init_shmem(int pid);
void exit_shmem(struct shmem_data *s);
int ack_shmem(struct shmem_data *s);
int wait_shmem(struct shmem_data *s);
int post_data_shmem(char *restrict shared_buffer, char *restrict data);
int get_data_shmem(char *restrict buffer, const char *restrict shared_buffer);

#endif /* __PAD_SHMEM_H__ */
