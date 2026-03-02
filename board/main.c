#include "shell.h"

#include "net.h"
#include "control.h"

int main(void) {
    init_net();
    start_service();

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run_forever(NULL, line_buf, sizeof(line_buf));

    return 0;
}
