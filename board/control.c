#include <stdio.h>
#include <stdlib.h>

#include "thread.h"
#include "ztimer.h"
#include "board.h"
#include "periph/gpio.h"

#include "net.h"

#include "control.h"

#define SEND_DURATION (30 * 60 * 1000)

struct ControlArg {
    struct SenderController controller;
    kernel_pid_t sender_pid;
};

static void sender_timeout(void *arg) {
    int *flag = arg;

    *flag = 0;
}

static void button_handler(void *arg) {
    struct ControlArg *typed_arg = arg;
    int *flag = typed_arg->controller.flag;
    thread_flags_t thread_flag = typed_arg->controller.thread_flags;
    kernel_pid_t sender_pid = typed_arg->sender_pid;

    static ztimer_t timer = { 0 };
    timer.callback = &sender_timeout;
    timer.arg = flag;

    if (!*flag) {
        *flag = 1;
        ztimer_set(ZTIMER_MSEC, &timer, SEND_DURATION);
        thread_flags_set(thread_get(sender_pid), thread_flag);
    } else {
        *flag = 0;
        ztimer_remove(ZTIMER_MSEC, &timer);
    }
}

void start_service(void) {
    // create listener thread
    static char listener_stack[THREAD_STACKSIZE_DEFAULT];
    kernel_pid_t pid = 0;
    pid = thread_create(
        listener_stack,
        sizeof(listener_stack),
        THREAD_PRIORITY_MAIN + 1,
        0,
        &listener,
        NULL,
        "listener");
    if (pid <= 0) {
        printf("ERROR: failed to create the listener thread, error code is %d\n", pid);
        abort();
    }

    // create sender thread
    static int flag = 0;
    static struct SenderController controller = {
        .thread_flags = 1, //  used to start sender, just a magic number
        .flag = &flag,     //  used to stop sender
    };
    static char sender_stack[THREAD_STACKSIZE_DEFAULT];
    pid = thread_create(
        sender_stack,
        sizeof(sender_stack),
        THREAD_PRIORITY_MAIN + 2,
        0,
        &sender,
        &controller,
        "sender");
    if (pid <= 0) {
        printf("ERROR: failed to create the sender thread, error code is %d\n", pid);
        abort();
    }

    // bind peripheral operations
    static struct ControlArg arg = { 0 };
    arg.controller = controller;
    arg.sender_pid = pid;
    if (gpio_init_int(BTN0_PIN, BTN0_MODE, GPIO_RISING, &button_handler, &arg)) {
        puts("ERROR: cannot bind the button");
        abort();
    }
}
