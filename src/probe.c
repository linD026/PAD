// TODO: Fix this
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200112L
#endif

#include <pad/pad.h>
#include <pad/action.h>
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

    if (info->prog_compiled[0] != '\0') {
        pr_debug("post path:%s\n", info->prog_compiled);
        post_data_shmem(s->shared->path, info->prog_compiled);
    }

    pr_debug("post enterpoint symbol:%s\n", info->enterpoint);
    post_data_shmem(s->shared->enterpoint, info->enterpoint);

    pr_debug("post symbol:%s\n", info->symbol);
    post_data_shmem(s->shared->symbol, info->symbol);

    pr_debug("post action:%s\n", act_table[info->action]);
    set_action_shmem(s, info->action);

    kill(info->target_pid, SIGTRAP);
    pr_debug("signal pid %d\n", info->target_pid);

    wait_shmem(s);
    pr_debug("received the ack\n");

    exit_shmem(s);
}
