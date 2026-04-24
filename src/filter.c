#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "filter.h"
#include "district.h"
#include "permissions.h"


 // AI-assisted section — see ai_usage.md for full details

// splits "field:operator:value" into its three parts
int parse_condition(const char *input, char *field, char *op, char *value)
{
    const char *first_colon = strchr(input, ':');
    if (!first_colon) return -1;

    int field_len = (int)(first_colon - input);
    strncpy(field, input, field_len);
    field[field_len] = '\0';

    const char *op_start = first_colon + 1;
    const char *second_colon = strchr(op_start, ':');
    if (!second_colon) return -1;

    int op_len = (int)(second_colon - op_start);
    strncpy(op, op_start, op_len);
    op[op_len] = '\0';

    strcpy(value, second_colon + 1);

    if (field[0] == '\0' || op[0] == '\0' || value[0] == '\0') return -1;

    return 0;
}

// returns 1 if report matches the condition, 0 otherwise
int match_condition(const Report *r, const char *field,
                    const char *op, const char *value)
{
    // string fields — only == and != supported
    if (strcmp(field, "category") == 0) {
        int eq = (strcmp(r->category, value) == 0);
        if (strcmp(op, "==") == 0) return eq;
        if (strcmp(op, "!=") == 0) return !eq;
        fprintf(stderr, "WARNING: operator '%s' not supported for field '%s'\n", op, field);
        return 0;
    }

    if (strcmp(field, "inspector") == 0) {
        int eq = (strcmp(r->inspector, value) == 0);
        if (strcmp(op, "==") == 0) return eq;
        if (strcmp(op, "!=") == 0) return !eq;
        fprintf(stderr, "WARNING: operator '%s' not supported for field '%s'\n", op, field);
        return 0;
    }

    /* numeric fields */
    long lhs = 0, rhs = atol(value);

    if (strcmp(field, "severity") == 0)
        lhs = (long)r->severity;
    else if (strcmp(field, "timestamp") == 0)
        lhs = (long)r->timestamp;
    else {
        fprintf(stderr, "WARNING: unknown filter field '%s'\n", field);
        return 0;
    }

    if (strcmp(op, "==") == 0) return lhs == rhs;
    if (strcmp(op, "!=") == 0) return lhs != rhs;
    if (strcmp(op, "<")  == 0) return lhs <  rhs;
    if (strcmp(op, "<=") == 0) return lhs <= rhs;
    if (strcmp(op, ">")  == 0) return lhs >  rhs;
    if (strcmp(op, ">=") == 0) return lhs >= rhs;

    fprintf(stderr, "WARNING: unknown operator '%s'\n", op);
    return 0;
}

//filter logic
int filter_reports(const char *district, const char **conditions, int n_conditions)
{
    char path[512];
    district_path(district, REPORTS_FILE, path, sizeof(path));

    if (perm_check(path, ROLE_INSPECTOR, "read") < 0) return -1;

    int fd = open(path, O_RDONLY);
    if (fd < 0) { perror(path); return -1; }

    // parse all conditions upfront to catch format errors early
    char fields[16][64], ops[16][8], values[16][128];
    for (int i = 0; i < n_conditions; i++) {
        if (parse_condition(conditions[i], fields[i], ops[i], values[i]) < 0) {
            fprintf(stderr, "ERROR: malformed condition '%s'\n", conditions[i]);
            close(fd);
            return -1;
        }
    }

    Report r;
    int found = 0;
    while (read(fd, &r, sizeof(Report)) == sizeof(Report)) {
        int match = 1;
        for (int i = 0; i < n_conditions; i++) {
            if (!match_condition(&r, fields[i], ops[i], values[i])) {
                match = 0;
                break;
            }
        }
        if (match) {
            report_print(&r);
            found++;
        }
    }

    close(fd);

    if (found == 0)
        printf("No reports matched the given conditions.\n");
    else
        printf("\n%d report(s) matched.\n", found);

    return 0;
}
