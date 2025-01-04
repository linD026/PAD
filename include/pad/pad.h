#ifndef __PAD_H__
#define __PAD_H__

#define FIXED_BUF_SIZE 256
#define CFLAGS_MAX_SIZE 16

/* act_table[] is in src/pad.c */
enum action_type {
    PAD_ACT_DUMP = 0,
    PAD_ACT_LOAD,
    PAD_ACT_UNLOAD,
    PAD_ACT_DEBUG,
    PAD_NR_ACTION,
};

struct core_info {
    struct /* compiler info*/ {
        char compiler[FIXED_BUF_SIZE];
        unsigned int nr_cflags;
        char cflags[CFLAGS_MAX_SIZE][FIXED_BUF_SIZE];
    };
    struct /* probe program */ {
        char program[FIXED_BUF_SIZE];
        char prog_compiled[FIXED_BUF_SIZE + FIXED_BUF_SIZE];
    };
    struct /* target info */ {
        char target[FIXED_BUF_SIZE];
        int target_pid;
        char symbol[FIXED_BUF_SIZE];
    };
    enum action_type action;
};

#endif /* __PAD_H__ */
