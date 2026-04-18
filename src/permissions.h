#ifndef PERMISSIONS_H
#define PERMISSIONS_H

#include <sys/stat.h>

//expected permission bits
#define PERM_DISTRICT_DIR   0750
#define PERM_REPORTS_DAT    0664
#define PERM_DISTRICT_CFG   0640
#define PERM_LOGGED         0644

#define ROLE_INSPECTOR  "inspector"
#define ROLE_MANAGER    "manager"

//set permissions on a path
int  perm_set(const char *path, mode_t mode);

//checks if role is read or write
int  perm_check(const char *path, const char *role, const char *action);

//convert permission strings
void perm_to_string(mode_t mode, char *buf);

//verify permission bits
int  perm_verify(const char *path, mode_t expected);

#endif