#include <pad/pad.h>
#include <pad/logs.h>
#include <pad/verifier.h>
#include <pad/probe.h>
#include <pad/utils.h>

#include <stdlib.h> /* for atoi() */
#include <errno.h>
#include <getopt.h>
#include <string.h> /* for memcpy() */

#undef pr_fmt
#define pr_fmt "[init] "

/*
 * The abstract:
 *
 *                                                    +-------------------+
 *                                                    | libpad: init/exit |
 *                            +----------+            +-------------------+
 *                            | verifier |            |
 *                            +----------+          +---------------------+
 * +-------------------+           |        load    | Target application  |
 * | robe program      |---+       |        unload  +---------------------+
 * +-------------------+   |  +----------+  debug   |                     |
 *                         +- | pad core | -------> | traceable function  |
 *                            +----------+          +---------------------+
 *                              |     |                       ^         |
 *                              |     |                       |         |
 *                              |     \                       |    [mmap/shmem]
 *                              |      +-------[ signal ]-----+         |
 *                              |    notification, is loaded or not?    |
 *                              \                                       |
 *                               +--[ shared mem: program location ]----+
 */

static struct core_info core_info = {
    .compiler = "/usr/bin/gcc",
    .cflags = { 
        { "-Wall" },
        { "-O2" },
    },
    .target_pid = -1,
    .nr_cflags = 2,
};

#define OPT_COMPILER 1
#define OPT_CFLAGS 2
#define OPT_PROGRAM 3
#define OPT_ENTERPOINT 4
#define OPT_TARGET 5
#define OPT_ACTION 6
#define OPT_SYMBOL 7
#define OPT_HELP 8

#define NR_OPTION 8
#define OPT_STRING "12345678"

struct opt_data {
    struct option options[NR_OPTION + 1];
};

static struct opt_data opt_data = {
    .options = { { "COMPILER", required_argument, 0, OPT_COMPILER },
                 { "CFLAGS", required_argument, 0, OPT_CFLAGS },
                 { "PROGRAM", required_argument, 0, OPT_PROGRAM },
                 { "ENTERPOINT", required_argument, 0, OPT_ENTERPOINT },
                 { "TARGET_PID", required_argument, 0, OPT_TARGET },
                 { "SYMBOL", required_argument, 0, OPT_SYMBOL },
                 { "ACTION", required_argument, 0, OPT_ACTION },
                 { "help", no_argument, 0, OPT_HELP },
                 { 0, 0, 0, 0 } },
};

#define ACT_ENTRY(act) [PAD_ACT_##act] = #act

/* clang-format off */
const char *act_table[] = {
    ACT_ENTRY(LOAD),
    ACT_ENTRY(UNLOAD),
    ACT_ENTRY(DEBUG)
};
/* clang-format on */

static int parse_action(void)
{
    char buffer[FIXED_BUF_SIZE] = { 0 };

    /* avoid access optarg with long operations for security... */
    strncpy(buffer, optarg, FIXED_BUF_SIZE);
    buffer[FIXED_BUF_SIZE - 1] = '\0';

    for (int i = 1; i < PAD_NR_ACTION; i++) {
        if (strncmp(act_table[i], buffer, strlen(act_table[i])) == 0) {
            core_info.action = i;
            return 0;
        }
    }

    core_info.action = PAD_ACT_DUMP;

    return -EINVAL;
}

static void set_option(int argc, char *argv[])
{
    int opt;
    int opt_index;
    int set_cflags = 0;

    while ((opt = getopt_long(argc, argv, OPT_STRING, opt_data.options,
                              &opt_index)) != -1) {
        switch (opt) {
        case OPT_COMPILER:
            if (WARN_ON(!optarg, "option: compiler is null (%s)",
                        opt_data.options[opt_index].name))
                break;
            strncpy(core_info.compiler, optarg, FIXED_BUF_SIZE);
            core_info.compiler[FIXED_BUF_SIZE - 1] = '\0';
            break;
        case OPT_CFLAGS:
            if (!set_cflags) {
                set_cflags = 1;
                core_info.nr_cflags = 0;
            }
            if (WARN_ON(!optarg, "option: cflags is null (%s)",
                        opt_data.options[opt_index].name))
                break;
            strncpy(core_info.cflags[core_info.nr_cflags], optarg,
                    FIXED_BUF_SIZE);
            core_info.cflags[core_info.nr_cflags++][FIXED_BUF_SIZE - 1] = '\0';
            BUG_ON(core_info.nr_cflags >= CFLAGS_MAX_SIZE, "overflow");
            break;
        case OPT_PROGRAM:
            if (WARN_ON(!optarg, "option: program is null (%s)",
                        opt_data.options[opt_index].name))
                break;
            strncpy(core_info.program, optarg, FIXED_BUF_SIZE);
            core_info.program[FIXED_BUF_SIZE - 1] = '\0';
            break;
        case OPT_ENTERPOINT:
            if (WARN_ON(!optarg, "option: enterpoint is null (%s)",
                        opt_data.options[opt_index].name))
                break;
            strncpy(core_info.enterpoint, optarg, FIXED_BUF_SIZE);
            core_info.enterpoint[FIXED_BUF_SIZE - 1] = '\0';
            break;
        case OPT_TARGET:
            if (WARN_ON(!optarg, "option: target is null (%s)",
                        opt_data.options[opt_index].name))
                break;
            core_info.target_pid = atoi(optarg);
            break;
        case OPT_SYMBOL:
            if (WARN_ON(!optarg, "option: symbol is null (%s)",
                        opt_data.options[opt_index].name))
                break;
            strncpy(core_info.symbol, optarg, FIXED_BUF_SIZE);
            core_info.symbol[FIXED_BUF_SIZE - 1] = '\0';
            break;
        case OPT_ACTION:
            BUG_ON(parse_action(), "unkown action");
            break;

        case OPT_HELP:
            printf("PAD - the userspace application debugger\n");
            printf("Usage: pad [options] file...\n");
            printf("Options:\n");
            printf("  %-16s %s\n", "--COMPILER",
                   "The compiler for building probe program.");
            printf("  %-16s %s\n", "--CFLAGS",
                   "The flag pass to the compiler.");
            printf("  %-16s %s\n", "--PROGRAM",
                   "The file of probe program to compile.");
            printf("  %-16s %s\n", "--ENTERPOINT",
                   "The symbol of the enterpoint for the probe program.");
            printf("  %-16s %s\n", "--TARGET_PID",
                   "The pid of process to probe.");
            printf("  %-16s %s\n", "--SYMBOL",
                   "The symbol of function want to probe.");
            printf("  %-16s %s\n", "--ACTION",
                   "The action of pad <LOAD|UNLOAD|DEBUG>.");
            printf("  %-16s %s\n", "--help", "Display this information.");

            exit(0);

        default:
            BUG_ON(1, "unkown option: %d", opt);
        }
    }
}

int main(int argc, char *argv[])
{
    set_option(argc, argv);

    pr_info("PAD core application for load/unload/debug\n");

    if (WARN_ON(!strlen(core_info.compiler), "unset compiler"))
        return -EINVAL;
    pr_info("compiler: %s\n", core_info.compiler);
    pr_info("flags:\n");
    for (int i = 0; i < core_info.nr_cflags; i++) {
        BUG_ON(!strlen(core_info.cflags[i]), "unset cflags");
        pr_info("    %s\n", core_info.cflags[i]);
    }

    if (WARN_ON(!strlen(core_info.program), "unset program"))
        return -EINVAL;
    pr_info("program file: %s\n", core_info.program);

    if (WARN_ON(!strlen(core_info.enterpoint), "unset enterpoint"))
        return -EINVAL;
    pr_info("enterpoint symbol: %s\n", core_info.enterpoint);

    if (WARN_ON(core_info.target_pid == -1, "unset program binary"))
        return -EINVAL;
    pr_info("target program pid: %d\n", core_info.target_pid);

    if (WARN_ON(core_info.action <= PAD_ACT_DUMP ||
                    core_info.action >= PAD_NR_ACTION,
                "core_info.action is corrupted:%d", core_info.action))
        return -EINVAL;

    pr_info("action: %s\n", act_table[core_info.action]);

    if (WARN_ON(!strlen(core_info.symbol), "unset program symbol"))
        return -EINVAL;
    pr_info("probe symbol code: %s\n", core_info.symbol);

    if (core_info.action == PAD_ACT_LOAD) {
        int ret = verify_and_compile_program(&core_info);
        if (unlikely(ret))
            goto out;
        probe_program(&core_info);
    } else if (core_info.action == PAD_ACT_UNLOAD) {
        probe_program(&core_info);
    } else if (core_info.action == PAD_ACT_DEBUG) {
        // TODO:
    }

out:
    return 0;
}
