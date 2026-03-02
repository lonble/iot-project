#pragma once

#include "thread_flags.h"

struct SenderController {
    thread_flags_t thread_flags;
    int *flag;
};

extern void init_net(void);
extern void *listener(void *arg);
extern void *sender(void *arg);
