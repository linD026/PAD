#ifndef __PAD_H__
#define __PAD_H__

#ifndef __PAD_ACTION_H__
#include "action.h"
#endif

#define FIXED_BUF_SIZE 256
#define CFLAGS_MAX_SIZE 16

struct core_info {
    struct /* compiler info*/ {
        char compiler[FIXED_BUF_SIZE];
        unsigned int nr_cflags;
        char cflags[CFLAGS_MAX_SIZE][FIXED_BUF_SIZE];
    };
    struct /* probe program */ {
        char program[FIXED_BUF_SIZE];
        char prog_compiled[FIXED_BUF_SIZE + FIXED_BUF_SIZE];
        char enterpoint[FIXED_BUF_SIZE];
    };
    struct /* target info */ {
        char target[FIXED_BUF_SIZE];
        int target_pid;
        char symbol[FIXED_BUF_SIZE];
    };
    enum action_type action;
};

#endif /* __PAD_H__ */
