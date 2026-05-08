#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include "monitor.h"

// flag set by SIGINT to exit the main loop
static volatile sig_atomic_t running = 1;

// SIGUSR1 handler — new report was added
static void handle_sigusr1(int sig)
{
    (void)sig;
    printf("[monitor] new report added to the system\n");
    fflush(stdout);
}

// SIGINT handler — shutdown 
static void handle_sigint(int sig)
{
    (void)sig;
    running = 0;
}

int main(void)
{
    // write PID to .monitor_pid 
    FILE *f = fopen(MONITOR_PID_FILE, "w");
    if (!f) {
        perror(MONITOR_PID_FILE);
        return 1;
    }
    fprintf(f, "%d\n", getpid());
    fclose(f);

    printf("[monitor] started with PID %d\n", getpid());
    fflush(stdout);

    // set up signal handlers using sigaction — not signal()
    struct sigaction sa_usr1, sa_int;

    sa_usr1.sa_handler = handle_sigusr1;
    sigemptyset(&sa_usr1.sa_mask);
    sa_usr1.sa_flags = SA_RESTART;
    sigaction(SIGUSR1, &sa_usr1, NULL);

    sa_int.sa_handler = handle_sigint;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = 0;
    sigaction(SIGINT, &sa_int, NULL);

    // wait for signals in a loop
    while (running)
        pause();

    // cleanup
    printf("[monitor] shutting down — removing %s\n", MONITOR_PID_FILE);
    fflush(stdout);
    unlink(MONITOR_PID_FILE);

    return 0;
}
