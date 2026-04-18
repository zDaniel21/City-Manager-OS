#ifndef DISTRICT_H
#define DISTRICT_H

// files and dirs for each district
#define REPORTS_FILE    "reports.dat"
#define CONFIG_FILE     "district.cfg"
#define LOG_FILE        "logged_district"
#define SYMLINK_PREFIX  "active_reports-"

#define DEFAULT_THRESHOLD 1

//checks if district and its files are ok
int  district_init(const char *district);

//builds a path
char *district_path(const char *district, const char *filename,
                    char *buf, int len);

//reads severity
int  district_get_threshold(const char *district);

//updates severity
int  district_set_threshold(const char *district, int value);

//create or refresh district symlink
int  district_update_symlink(const char *district);

#endif