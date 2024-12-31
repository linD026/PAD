#ifndef __PAD_H__
#define __PAD_H__

#define FIXED_BUF_SIZE 128
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
    char compiler[FIXED_BUF_SIZE];
    struct {
        unsigned int nr_cflags;
        char cflags[CFLAGS_MAX_SIZE][FIXED_BUF_SIZE];
    };
    char program[FIXED_BUF_SIZE];
    char target[FIXED_BUF_SIZE];
    int target_pid;
    char symbol[FIXED_BUF_SIZE];
    enum action_type action;
};

struct prog_struct {
    char name[FIXED_BUF_SIZE];
};

#endif /* __PAD_H__ */
