#include <pad/pad.h>
#include <pad/logs.h>
#include <pad/verifier.h>
#include <pad/probe.h>
#include <pad/utils.h>

#include <errno.h>
#include <getopt.h>
#include <string.h>

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
 * +-------------------+           |                | Target application  |
 * | pad probe program |---+       |        debug   +---------------------+
 * +-------------------+   |  +----------+  unload  |                     |
 *                         +- | pad core | -------> | breakpoint function |
 *                            +----------+   load   +---------------------+
 *                              |     |                       ^         |
 *                              |     |                       |         |
 *                              |     \                       |    [mmap/shmem]
 *                              |      +-------[ signal ]-----+         |
 *                              |    notification, is loaded or not?    |
 *                              \                                       |
 *                               +--[ shared mem: program location ]----+
 */

static struct core_info core_info = {
    .compiler = "gcc",
    .cflags = { 
        { "-Wall" },
        { "-O2" },
    },
    .nr_cflags = 2,
};

#define OPT_COMPILER 0
#define OPT_CFLAGS 1
#define OPT_PROGRAM 2
#define OPT_TARGET 3
#define OPT_ACTION 4
#define OPT_SYMBOL 5

#define NR_OPTION 6
#define OPT_STRING "012345"

struct opt_data {
    struct option options[NR_OPTION];
};

static struct opt_data opt_data = {
    .options = { { "CC", optional_argument, 0, OPT_COMPILER },
                 { "CFLAGS", optional_argument, 0, OPT_CFLAGS },
                 { "PROGRAM", required_argument, 0, OPT_PROGRAM },
                 { "TARGET", required_argument, 0, OPT_TARGET },
                 { "SYMBOL", required_argument, 0, OPT_SYMBOL },
                 { "ACTION", required_argument, 0, OPT_ACTION } },
};

#define ACT_ENTRY(act) [PAD_ACT_##act] = #act

/* clang-format off */
static const char *act_table[] = {
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

static void pr_action(void)
{
    BUG_ON(core_info.action <= PAD_ACT_DUMP ||
               core_info.action >= PAD_NR_ACTION,
           "core_info.action is corrupted:%d", core_info.action);

    pr_info("action: %s\n", act_table[core_info.action]);
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
            strncpy(core_info.compiler, optarg, FIXED_BUF_SIZE);
            core_info.compiler[FIXED_BUF_SIZE - 1] = '\0';
            break;
        case OPT_CFLAGS:
            if (!set_cflags) {
                set_cflags = 1;
                core_info.nr_cflags = 0;
            }
            strncpy(core_info.cflags[core_info.nr_cflags], optarg,
                    FIXED_BUF_SIZE);
            core_info.cflags[core_info.nr_cflags++][FIXED_BUF_SIZE - 1] = '\0';
            BUG_ON(core_info.nr_cflags >= CFLAGS_MAX_SIZE, "overflow");
            break;
        case OPT_PROGRAM:
            strncpy(core_info.program, optarg, FIXED_BUF_SIZE);
            core_info.program[FIXED_BUF_SIZE - 1] = '\0';
            break;
        case OPT_TARGET:
            strncpy(core_info.target, optarg, FIXED_BUF_SIZE);
            core_info.target[FIXED_BUF_SIZE - 1] = '\0';
            break;
        case OPT_SYMBOL:
            strncpy(core_info.symbol, optarg, FIXED_BUF_SIZE);
            core_info.symbol[FIXED_BUF_SIZE - 1] = '\0';
            break;
        case OPT_ACTION:
            BUG_ON(parse_action(), "unkown action");
            break;

        default:
            BUG_ON(1, "unkown option: %d", opt);
        }
    }
}

int main(int argc, char *argv[])
{
    pr_info("PAD core application for load/unload/debug\n");

    set_option(argc, argv);

    BUG_ON(!strlen(core_info.compiler), "unset compiler");
    pr_info("compiler: %s\n", core_info.compiler);
    pr_info("flags:\n");
    for (int i = 0; i < core_info.nr_cflags; i++) {
        BUG_ON(!strlen(core_info.cflags[i]), "unset cflags");
        pr_info("    %s\n", core_info.cflags[i]);
    }

    BUG_ON(!strlen(core_info.program), "unset program");
    pr_info("program file: %s\n", core_info.program);

    BUG_ON(!strlen(core_info.target), "unset program binary");
    pr_info("target program binary: %s\n", core_info.target);

    if (core_info.action == PAD_ACT_LOAD) {
        BUG_ON(!strlen(core_info.symbol), "unset program symbol");
        pr_info("probe symbol code: %s\n", core_info.symbol);
    }

    pr_action();

    verify_and_compile_program(&core_info);
    probe_handler(&core_info);

    return 0;
}
