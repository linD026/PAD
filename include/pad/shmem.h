#ifndef __PAD_SHMEM_H__
#define __PAD_SHMEM_H__

#ifndef FIXED_BUF_SIZE
#define FIXED_BUF_SIZE 128
#endif

struct shmem_data {
    const char path_name[FIXED_BUF_SIZE];
    int fd;
};

struct shmem_data *init_shmem(int pid);
void exit_shmem(struct shmem_data *s);
int ack_shmem(struct shmem_data *s);
int wait_shmem(struct shmem_data *s);
int get_data_shmem(struct shmem_data *s);

#endif /* __PAD_SHMEM_H__ */
