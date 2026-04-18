#include <stdio.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include "log.h"
#include "district.h"

int log_action(const char *district, const char *user,
               const char *role, const char *action)
{
    char path[512];
    district_path(district, LOG_FILE, path, sizeof(path));

    int fd = open(path, O_WRONLY | O_APPEND);
    if (fd < 0) {
        perror(path);
        return -1;
    }

    time_t now = time(NULL);
    char line[256];
    int len = snprintf(line, sizeof(line),
                       "%ld\t%s\t%s\t%s\n",
                       (long)now, user, role, action);

    write(fd, line, len);
    close(fd);
    return 0;
}