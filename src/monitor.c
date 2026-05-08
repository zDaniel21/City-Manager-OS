#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include "monitor.h"
#include "log.h"

/* read monitor PID from .monitor_pid and send SIGUSR1 */
void monitor_notify(const char *district, const char *user, const char *role)
{
    FILE *f = fopen(MONITOR_PID_FILE, "r");
    if (!f) {
        fprintf(stderr, "monitor not running — no .monitor_pid found\n");
        log_action(district, user, role, "add: monitor could not be informed");
        return;
    }

    pid_t pid;
    if (fscanf(f, "%d", &pid) != 1) {
        fprintf(stderr, "ERROR: could not read PID from .monitor_pid\n");
        fclose(f);
        log_action(district, user, role, "add: monitor could not be informed");
        return;
    }
    fclose(f);

    if (kill(pid, SIGUSR1) < 0) {
        perror("kill");
        log_action(district, user, role, "add: monitor could not be informed");
        return;
    }

    log_action(district, user, role, "add: monitor notified via SIGUSR1");
}
