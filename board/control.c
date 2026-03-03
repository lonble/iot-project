#include <stdio.h>
#include <stdlib.h>

#include "thread.h"
#include "ztimer.h"
#include "ztimer/periodic.h"
#include "board.h"
#include "periph/gpio.h"

#include "net.h"

#include "control.h"

#define SEND_DURATION (30 * 60 * 1000)
#define SEND_INTERVAL 5 // packets will be droped if the frequency is higher
#define WATCH_INTERVAL 50
#define EXPECTED_PACKETS (WATCH_INTERVAL / SEND_INTERVAL)
#define HTRESHOLD 0.5
#define NEEDED_PACKETS ((int)(EXPECTED_PACKETS * HTRESHOLD))

struct ControlArg {
    struct SenderController controller;
    kernel_pid_t sender_pid;
};

static bool watcher_routine(void *arg) {
    int *indicator = arg;

    int packets = *indicator;
    *indicator = 0;
    if (packets >= NEEDED_PACKETS) {
        packets = NEEDED_PACKETS;
    }
    static int phase = 0;
    phase += packets;
    if (phase >= NEEDED_PACKETS) {
        phase -= NEEDED_PACKETS;
        LED1_ON;
    } else {
        LED1_OFF;
    }

    return ZTIMER_PERIODIC_KEEP_GOING;
}

static void sender_timeout(void *arg) {
    int *flag = arg;

    *flag = 0;
    LED0_OFF;
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
        LED0_ON;
        ztimer_set(ZTIMER_MSEC, &timer, SEND_DURATION);
        thread_flags_set(thread_get(sender_pid), thread_flag);
    } else {
        *flag = 0;
        LED0_OFF;
        ztimer_remove(ZTIMER_MSEC, &timer);
    }
}

void start_service(void) {
    // create listener thread
    static int indicator = 0;
    static ztimer_periodic_t watcher;
    ztimer_periodic_init(
        ZTIMER_MSEC,
        &watcher,
        &watcher_routine,
        &indicator,
        WATCH_INTERVAL);
    static char listener_stack[THREAD_STACKSIZE_DEFAULT];
    kernel_pid_t pid = 0;
    pid = thread_create(
        listener_stack,
        sizeof(listener_stack),
        THREAD_PRIORITY_MAIN + 1,
        0,
        &listener,
        &indicator,
        "listener");
    if (pid <= 0) {
        printf("ERROR: failed to create the listener thread, error code is %d\n", pid);
        abort();
    }

    // start the watcher
    ztimer_periodic_start(&watcher);

    // create sender thread
    static int flag = 0;
    static struct SenderController controller = {
        .thread_flags = 1, //  used to start sender, just a magic number
        .flag = &flag,     //  used to stop sender
        .interval = SEND_INTERVAL,
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

    // bind button operations
    static struct ControlArg arg = { 0 };
    arg.controller = controller;
    arg.sender_pid = pid;
    if (gpio_init_int(BTN0_PIN, BTN0_MODE, GPIO_RISING, &button_handler, &arg)) {
        puts("ERROR: cannot bind the button");
        abort();
    }
}
