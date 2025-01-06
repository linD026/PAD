#ifndef __PAD_ACTION_H__
#define __PAD_ACTION_H__

/* act_table[] is in src/pad.c */
enum action_type {
    PAD_ACT_DUMP = 0,
    PAD_ACT_LOAD,
    PAD_ACT_UNLOAD,
    PAD_ACT_DEBUG,
    PAD_NR_ACTION,
};

extern const char *act_table[];

#endif /* __PAD_ACTION_H__ */
