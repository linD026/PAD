#ifndef __PAD_SHMEM_H__
#define __PAD_SHMEM_H__

#ifndef __PAD_ACTION_H__
#include "action.h"
#endif

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
    unsigned int action;
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

void cleanup_shmem(struct shmem_data *s);

int post_data_shmem(char *restrict shared_buffer, char *restrict data);
int get_data_shmem(char *restrict buffer, const char *restrict shared_buffer);

int set_action_shmem(struct shmem_data *s, enum action_type act);
enum action_type get_action_shmem(struct shmem_data *s);

#endif /* __PAD_SHMEM_H__ */
