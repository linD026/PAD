#define _GNU_SOURCE
#include <pad/pad.h>
#include <pad/logs.h>
#include <pad/utils.h>

#include <stdio.h> /* for fopen() */
#include <spawn.h>
#include <wait.h>
#include <string.h>
#include <errno.h>

#undef pr_fmt
#define pr_fmt "[verifier] "

static int lw_system(char *restrict exec, char *argv[])
{
    int ret = 0;
    pid_t child_pid = 0;
    posix_spawnattr_t attr;
    extern char **environ;

    /* TODO: Add error handling instead of using the BUG_ON. */
    BUG_ON(posix_spawnattr_init(&attr), "init attr");
    BUG_ON(posix_spawnattr_setflags(&attr, POSIX_SPAWN_USEVFORK), "set attr");

    ret = posix_spawn(&child_pid, argv[0], NULL, &attr, argv, environ);

    if (WARN_ON(ret, "posix_spawn failed")) {
        pr_info("posix_spawn:%d pid:%d error: %s\n",
                ret, child_pid, strerror(ret));
        return -ECHILD;
    }

    ret = waitpid(child_pid, &ret, 0);
    WARN_ON(ret == -1, "waitpid");

    return ret != -1 ? 0 : -EINVAL;
}

static int compile_program(struct core_info *info)
{
    char *argv[FIXED_BUF_SIZE] = { 0 };
    int i = 0, argc = 0, ret = 0;

    sprintf(info->prog_compiled, "%s.so", info->program);

    pr_info("compile: %s\n", info->prog_compiled);

    argv[argc++] = info->compiler;
    argv[argc++] = "-o";
    argv[argc++] = info->prog_compiled;
    argv[argc++] = info->program;

    i = 0;
    for (char *iter = info->cflags[i]; i < info->nr_cflags;
         iter = info->cflags[++i])
        argv[argc++] = iter;

    argv[argc++] = "-shared";
    argv[argc++] = NULL;

    WARN_ON(argc >= FIXED_BUF_SIZE, "overflow");

    /* Return 0, if the command succeed. */
    ret = lw_system(info->compiler, argv);
    if (WARN_ON(ret, "compile(\"%s\") return %d error:%s",
            info->prog_compiled, ret, strerror(ret))) {
        for (i = 0; i < argc; i++)
            pr_info("argv[%d]: %s\n", i, argv[i]);
    }

    return ret;
}

static int get_target_info(struct core_info *info)
{
    FILE *fp = NULL;
    size_t size = 0;
    int ret = 0;

    snprintf(info->target, FIXED_BUF_SIZE, "/proc/%d/comm", info->target_pid);
    info->target[FIXED_BUF_SIZE - 1] = '\0';

    fp = fopen(info->target, "r");

    if (WARN_ON(!fp, "%s doesn't existed", info->target))
        return -EINVAL;

    size = fread(info->target, FIXED_BUF_SIZE, 1, fp);
    ret = ferror(fp);
    if (WARN_ON(ret, "fread error: read size:%zu, ret:%d", size, ret))
        return -ret;

    fclose(fp);

    return 0;
}

int verify_and_compile_program(struct core_info *info)
{
    int ret = 0;

    ret = get_target_info(info);
    if (unlikely(ret))
        return ret;

    pr_info("target pid binary:\n%s\n", info->target);

    compile_program(info);

    return 0;
}
