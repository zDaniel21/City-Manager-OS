#ifndef REPORT_H
#define REPORT_H

#include <time.h>

#define INSPECTOR_NAME_LEN  64
#define CATEGORY_LEN        32
#define DESCRIPTION_LEN     256

// severity levels
#define SEV_MINOR     1
#define SEV_MODERATE  2
#define SEV_CRITICAL  3

typedef struct {
    int     id;
    char    inspector[INSPECTOR_NAME_LEN];
    double  latitude;
    double  longitude;
    char    category[CATEGORY_LEN];
    int     severity;
    time_t  timestamp;
    char    description[DESCRIPTION_LEN];
} Report;

int  report_add(const char *district, const char *user, double lat, double lon,
                const char *category, int severity, const char *description);
int  report_list(const char *district);
int  report_view(const char *district, int report_id);
int  report_remove(const char *district, int report_id);
void report_print(const Report *r);

#endif