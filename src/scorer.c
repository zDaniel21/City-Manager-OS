#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "report.h"
#include "district.h"

#define MAX_INSPECTORS 64

typedef struct {
    char name[INSPECTOR_NAME_LEN];
    int  score;
} InspectorScore;

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: scorer <district>\n");
        return 1;
    }

    const char *district = argv[1];
    char path[512];
    district_path(district, REPORTS_FILE, path, sizeof(path));

    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "scorer: cannot open %s\n", path);
        return 1;
    }

    InspectorScore scores[MAX_INSPECTORS];
    int n_inspectors = 0;

    Report r;
    while (read(fd, &r, sizeof(Report)) == sizeof(Report)) {
        // find existing inspector or add new one
        int found = 0;
        for (int i = 0; i < n_inspectors; i++) {
            if (strcmp(scores[i].name, r.inspector) == 0) {
                scores[i].score += r.severity;
                found = 1;
                break;
            }
        }
        if (!found && n_inspectors < MAX_INSPECTORS) {
            strncpy(scores[n_inspectors].name, r.inspector, INSPECTOR_NAME_LEN - 1);
            scores[n_inspectors].score = r.severity;
            n_inspectors++;
        }
    }

    close(fd);

    // print results to stdout — hub reads this via pipe
    printf("District: %s\n", district);
    for (int i = 0; i < n_inspectors; i++)
        printf("  %s: %d\n", scores[i].name, scores[i].score);

    return 0;
}