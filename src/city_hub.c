#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "monitor.h"

#define MAX_LINE 512

// hub_mon child — creates pipe, forks monitor_reports, reads its output
static void run_hub_mon(void)
{
    int pipefd[2];
    if (pipe(pipefd) < 0) {
        perror("pipe");
        exit(1);
    }

    pid_t mon_pid = fork();
    if (mon_pid < 0) {
        perror("fork");
        exit(1);
    }

    if (mon_pid == 0) {
        // child — redirect stdout to write end of pipe
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);

        execl("./monitor_reports", "monitor_reports", NULL);
        perror("execl monitor_reports");
        exit(1);
    }

    // parent hub_mon — read from pipe and display messages
    close(pipefd[1]);

    char line[MAX_LINE];
    FILE *pipe_read = fdopen(pipefd[0], "r");
    if (!pipe_read) {
        perror("fdopen");
        exit(1);
    }

    while (fgets(line, sizeof(line), pipe_read)) {
        // strip newline
        line[strcspn(line, "\n")] = '\0';

        if (strncmp(line, "MSG:", 4) == 0) {
            printf("[hub] monitor says: %s\n", line + 4);
        } else if (strncmp(line, "ERR:", 4) == 0) {
            printf("[hub] monitor error: %s\n", line + 4);
        } else {
            printf("[hub] %s\n", line);
        }
        fflush(stdout);
    }

    fclose(pipe_read);

    // monitor ended
    int status;
    waitpid(mon_pid, &status, 0);
    printf("[hub] monitor process has ended\n");
    fflush(stdout);
    exit(0);
}

// calculate_scores — spawn a scorer per district, collect output via pipes
static void run_calculate_scores(char **districts, int n_districts)
{
    for (int i = 0; i < n_districts; i++) {
        int pipefd[2];
        if (pipe(pipefd) < 0) {
            perror("pipe");
            continue;
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            continue;
        }

        if (pid == 0) {
            // child scorer — redirect stdout to pipe
            close(pipefd[0]);
            dup2(pipefd[1], STDOUT_FILENO);
            close(pipefd[1]);

            execl("./scorer", "scorer", districts[i], NULL);
            perror("execl scorer");
            exit(1);
        }

        // parent — read scorer output from pipe
        close(pipefd[1]);

        char line[MAX_LINE];
        FILE *pipe_read = fdopen(pipefd[0], "r");
        if (!pipe_read) {
            perror("fdopen");
            continue;
        }

        while (fgets(line, sizeof(line), pipe_read))
            printf("%s", line);

        fclose(pipe_read);
        waitpid(pid, NULL, 0);
    }
}

int main(void)
{
    char input[MAX_LINE];

    printf("city_hub started. Commands: start_monitor, calculate_scores <districts...>, exit\n");

    while (1) {
        printf("hub> ");
        fflush(stdout);

        if (!fgets(input, sizeof(input), stdin))
            break;

        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "exit") == 0)
            break;

        if (strcmp(input, "start_monitor") == 0) {
            pid_t hub_mon_pid = fork();
            if (hub_mon_pid < 0) {
                perror("fork");
                continue;
            }
            if (hub_mon_pid == 0) {
                // child becomes hub_mon
                run_hub_mon();
                exit(0);
            }
            printf("[hub] monitor started in background (hub_mon PID %d)\n", hub_mon_pid);
            continue;
        }

        if (strncmp(input, "calculate_scores", 16) == 0) {
            // parse district list from rest of input
            char *token;
            char *districts[32];
            int n = 0;

            char *rest = input + 16;
            token = strtok(rest, " ");
            while (token && n < 32) {
                districts[n++] = token;
                token = strtok(NULL, " ");
            }

            if (n == 0) {
                printf("Usage: calculate_scores <district1> [district2...]\n");
                continue;
            }

            run_calculate_scores(districts, n);
            continue;
        }

        printf("Unknown command: %s\n", input);
    }

    printf("[hub] shutting down\n");
    return 0;
}