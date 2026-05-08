#ifndef MONITOR_H
#define MONITOR_H

#define MONITOR_PID_FILE ".monitor_pid"

/* read PID from .monitor_pid and send SIGUSR1 to notify monitor.
   logs result to district log file. */
void monitor_notify(const char *district, const char *user, const char *role);

#endif
