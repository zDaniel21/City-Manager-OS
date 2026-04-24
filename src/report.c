#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include "report.h"
#include "district.h"
#include "permissions.h"

// print a single report to stdout
void report_print(const Report *r)
{
    char timebuf[64];
    struct tm *tm_info = localtime(&r->timestamp);
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", tm_info);

    printf("-----------------------------\n");
    printf("ID          : %d\n",   r->id);
    printf("Inspector   : %s\n",   r->inspector);
    printf("Latitude    : %.6f\n", r->latitude);
    printf("Longitude   : %.6f\n", r->longitude);
    printf("Category    : %s\n",   r->category);
    printf("Severity    : %d\n",   r->severity);
    printf("Timestamp   : %s\n",   timebuf);
    printf("Description : %s\n",   r->description);
}

// append a new report to reports.dat
int report_add(const char *district, const char *user,
               double lat, double lon,
               const char *category, int severity,
               const char *description)
{
    char path[512];
    district_path(district, REPORTS_FILE, path, sizeof(path));

    if (perm_check(path, ROLE_INSPECTOR, "write") < 0) return -1;

    int fd = open(path, O_RDWR);
    if (fd < 0) { perror(path); return -1; }

    /* determine next ID from current record count */
    struct stat st;
    fstat(fd, &st);
    int count = (int)(st.st_size / sizeof(Report));

    Report r;
    memset(&r, 0, sizeof(r));
    r.id        = count + 1;
    r.latitude  = lat;
    r.longitude = lon;
    r.severity  = severity;
    r.timestamp = time(NULL);
    strncpy(r.inspector,   user,        INSPECTOR_NAME_LEN - 1);
    strncpy(r.category,    category,    CATEGORY_LEN - 1);
    strncpy(r.description, description, DESCRIPTION_LEN - 1);

    lseek(fd, 0, SEEK_END);
    if (write(fd, &r, sizeof(Report)) != sizeof(Report)) {
        perror("write");
        close(fd);
        return -1;
    }

    close(fd);
    printf("Report #%d added to district '%s'\n", r.id, district);
    return 0;
}

// list all reports and print file metadata
int report_list(const char *district)
{
    char path[512];
    district_path(district, REPORTS_FILE, path, sizeof(path));

    if (perm_check(path, ROLE_INSPECTOR, "read") < 0) return -1;

    struct stat st;
    if (stat(path, &st) < 0) { perror(path); return -1; }

    char perm_str[10];
    char timebuf[64];
    perm_to_string(st.st_mode & 0777, perm_str);
    struct tm *tm_info = localtime(&st.st_mtime);
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", tm_info);

    printf("File      : %s\n", path);
    printf("Perms     : %s\n", perm_str);
    printf("Size      : %ld bytes\n", (long)st.st_size);
    printf("Modified  : %s\n", timebuf);

    int count = (int)(st.st_size / sizeof(Report));
    printf("Records   : %d\n\n", count);

    if (count == 0) {
        printf("No reports in district '%s'\n", district);
        return 0;
    }

    int fd = open(path, O_RDONLY);
    if (fd < 0) { perror(path); return -1; }

    Report r;
    while (read(fd, &r, sizeof(Report)) == sizeof(Report))
        report_print(&r);

    close(fd);
    return 0;
}

// view a single report by ID 
int report_view(const char *district, int report_id)
{
    char path[512];
    district_path(district, REPORTS_FILE, path, sizeof(path));

    if (perm_check(path, ROLE_INSPECTOR, "read") < 0) return -1;

    int fd = open(path, O_RDONLY);
    if (fd < 0) { perror(path); return -1; }

    Report r;
    int found = 0;
    while (read(fd, &r, sizeof(Report)) == sizeof(Report)) {
        if (r.id == report_id) {
            report_print(&r);
            found = 1;
            break;
        }
    }

    close(fd);

    if (!found) {
        fprintf(stderr, "ERROR: Report #%d not found in district '%s'\n",
                report_id, district);
        return -1;
    }
    return 0;
}

// remove a report by ID — shift subsequent records left, then truncate
int report_remove(const char *district, int report_id)
{
    char path[512];
    district_path(district, REPORTS_FILE, path, sizeof(path));

    if (perm_check(path, ROLE_MANAGER, "write") < 0) return -1;

    int fd = open(path, O_RDWR);
    if (fd < 0) { perror(path); return -1; }

    struct stat st;
    fstat(fd, &st);
    int count = (int)(st.st_size / sizeof(Report));

    // find the index of the record to remove
    int target_idx = -1;
    Report r;
    for (int i = 0; i < count; i++) {
        lseek(fd, (off_t)i * sizeof(Report), SEEK_SET);
        read(fd, &r, sizeof(Report));
        if (r.id == report_id) { target_idx = i; break; }
    }

    if (target_idx < 0) {
        fprintf(stderr, "ERROR: Report #%d not found in district '%s'\n",
                report_id, district);
        close(fd);
        return -1;
    }

    //shift all records after target one position earlier 
    for (int i = target_idx + 1; i < count; i++) {
        lseek(fd, (off_t)i * sizeof(Report), SEEK_SET);
        read(fd, &r, sizeof(Report));
        lseek(fd, (off_t)(i - 1) * sizeof(Report), SEEK_SET);
        write(fd, &r, sizeof(Report));
    }

    ftruncate(fd, (off_t)(count - 1) * sizeof(Report));
    close(fd);

    printf("Report #%d removed from district '%s'\n", report_id, district);
    return 0;
}
