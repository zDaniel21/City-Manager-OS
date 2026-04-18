#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include "district.h"
#include "permissions.h"

//build path into filename
char *district_path(const char *district, const char *filename,
                    char *buf, int len)
{
    snprintf(buf, len, "%s/%s", district, filename);
    return buf;
}

//create a file at path and set perms
static int create_file_if_missing(const char *path, mode_t mode)
{
    struct stat st;
    if (stat(path, &st) == 0)
        return perm_set(path, mode);

    int fd = open(path, O_CREAT | O_WRONLY, mode);
    if (fd < 0) {
        perror(path);
        return -1;
    }
    close(fd);
    return perm_set(path, mode);
}

//init district dir and all needed files
int district_init(const char *district)
{
    struct stat st;
    char path[512];

    if (stat(district, &st) < 0) {
        if (mkdir(district, PERM_DISTRICT_DIR) < 0) {
            perror(district);
            return -1;
        }
    }
    perm_set(district, PERM_DISTRICT_DIR);

    district_path(district, REPORTS_FILE, path, sizeof(path));
    if (create_file_if_missing(path, PERM_REPORTS_DAT) < 0) return -1;

    district_path(district, CONFIG_FILE, path, sizeof(path));
    int cfg_exists = (stat(path, &st) == 0);
    if (create_file_if_missing(path, PERM_DISTRICT_CFG) < 0) return -1;
    if (!cfg_exists) {
        int fd = open(path, O_WRONLY);
        if (fd >= 0) {
            char buf[32];
            int len = snprintf(buf, sizeof(buf), "threshold=%d\n", DEFAULT_THRESHOLD);
            write(fd, buf, len);
            close(fd);
        }
    }

    district_path(district, LOG_FILE, path, sizeof(path));
    if (create_file_if_missing(path, PERM_LOGGED) < 0) return -1;

    district_update_symlink(district);

    return 0;
}

//read threshold value
int district_get_threshold(const char *district)
{
    char path[512];
    district_path(district, CONFIG_FILE, path, sizeof(path));

    FILE *f = fopen(path, "r");
    if (!f) { perror(path); return -1; }

    int value = DEFAULT_THRESHOLD;
    fscanf(f, "threshold=%d", &value);
    fclose(f);
    return value;
}

//write new threshold 
int district_set_threshold(const char *district, int value)
{
    char path[512];
    district_path(district, CONFIG_FILE, path, sizeof(path));

    //verify perm bits are still 640 before writing
    if (perm_verify(path, PERM_DISTRICT_CFG) < 0) return -1;

    int fd = open(path, O_WRONLY | O_TRUNC);
    if (fd < 0) { perror(path); return -1; }

    char buf[32];
    int len = snprintf(buf, sizeof(buf), "threshold=%d\n", value);
    write(fd, buf, len);
    close(fd);
    return 0;
}

//create or refresh report symlink
int district_update_symlink(const char *district)
{
    char link_name[256];
    char target[512];
    struct stat lst;

    snprintf(link_name, sizeof(link_name), "%s%s", SYMLINK_PREFIX, district);
    snprintf(target,    sizeof(target),    "%s/%s", district, REPORTS_FILE);

    //remove existing link 
    if (lstat(link_name, &lst) == 0)
        unlink(link_name);

    if (symlink(target, link_name) < 0) {
        perror(link_name);
        return -1;
    }
    return 0;
}