// TODO: Fix this
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200112L
#endif

#include <pad/pad.h>
#include <pad/probe.h>
#include <pad/logs.h>
#include <pad/shmem.h>

#include <signal.h>

#undef pr_fmt
#define pr_fmt "[probe] "

void probe_program(struct core_info *info)
{
    struct shmem_data *s = NULL;

    s = init_shmem(info->target_pid);
    if (WARN_ON(!s, "init_shmem failed"))
        return;

    pr_info("post data:%s\n", info->prog_compiled);
    post_data_shmem(s->shared->path, info->prog_compiled);
    pr_info("post data:%s\n", info->symbol);
    post_data_shmem(s->shared->symbol, info->symbol);

    kill(info->target_pid, SIGTRAP);
    pr_info("signal pid %d\n", info->target_pid);

    wait_shmem(s);
    pr_info("received the ack\n");

    exit_shmem(s);
}
