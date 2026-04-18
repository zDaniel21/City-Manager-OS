#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "permissions.h"

//set permissions on path
int perm_set(const char *path, mode_t mode)
{
    if (chmod(path, mode) < 0) {
        perror(path);
        return -1;
    }
    return 0;
}

//convert permission bits to chars
void perm_to_string(mode_t mode, char *buf)
{
    //owner
    buf[0] = (mode & S_IRUSR) ? 'r' : '-';
    buf[1] = (mode & S_IWUSR) ? 'w' : '-';
    buf[2] = (mode & S_IXUSR) ? 'x' : '-';
    //group
    buf[3] = (mode & S_IRGRP) ? 'r' : '-';
    buf[4] = (mode & S_IWGRP) ? 'w' : '-';
    buf[5] = (mode & S_IXGRP) ? 'x' : '-';
    //other
    buf[6] = (mode & S_IROTH) ? 'r' : '-';
    buf[7] = (mode & S_IWOTH) ? 'w' : '-';
    buf[8] = (mode & S_IXOTH) ? 'x' : '-';
    buf[9] = '\0';
}

//verify permissions if they match
int perm_verify(const char *path, mode_t expected)
{
    struct stat st;
    char actual_str[10], expected_str[10];

    if (stat(path, &st) < 0) {
        perror(path);
        return -1;
    }

    mode_t actual = st.st_mode & 0777;
    if (actual != expected) {
        perm_to_string(actual,   actual_str);
        perm_to_string(expected, expected_str);
        fprintf(stderr,
            "ERROR: %s has permissions %s but expected %s — refusing operation\n",
            path, actual_str, expected_str);
        return -1;
    }
    return 0;
}

//check if role has the right access on path
int perm_check(const char *path, const char *role, const char *action)
{
    struct stat st;

    if (stat(path, &st) < 0) {
        perror(path);
        return -1;
    }

    mode_t mode = st.st_mode;
    int is_manager = (strcmp(role, ROLE_MANAGER) == 0);
    int is_read    = (strcmp(action, "read")  == 0);
    int is_write   = (strcmp(action, "write") == 0);
    int allowed    = 0;

    if (is_manager) {
        if (is_read  && (mode & S_IRUSR)) allowed = 1;
        if (is_write && (mode & S_IWUSR)) allowed = 1;
    } else {
        //inspector - group
        if (is_read  && (mode & S_IRGRP)) allowed = 1;
        if (is_write && (mode & S_IWGRP)) allowed = 1;
    }

    if (!allowed) {
        fprintf(stderr,
            "ERROR: role '%s' does not have %s access on %s\n",
            role, action, path);
        return -1;
    }
    return 0;
}