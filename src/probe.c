#include <pad/pad.h>
#include <pad/probe.h>
#include <pad/logs.h>

#undef pr_fmt
#define pr_fmt "[probe] "

static void load_program(struct core_info *info)
{
    pr_info("%s\n", __func__);
}

void probe_handler(struct core_info *info)
{
    pr_info("%s\n", __func__);
}
