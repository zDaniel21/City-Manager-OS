#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "report.h"
#include "district.h"
#include "permissions.h"
#include "filter.h"
#include "log.h"

static void usage(const char *prog)
{
    fprintf(stderr,
        "Usage:\n"
        "  %s --role <manager|inspector> --user <name> --add <district>\n"
        "  %s --role <manager|inspector> --user <name> --list <district>\n"
        "  %s --role <manager|inspector> --user <name> --view <district> <id>\n"
        "  %s --role manager             --user <name> --remove_report <district> <id>\n"
        "  %s --role manager             --user <name> --update_threshold <district> <value>\n"
        "  %s --role <manager|inspector> --user <name> --filter <district> <cond> [cond...]\n",
        prog, prog, prog, prog, prog, prog);
}

int main(int argc, char *argv[])
{
    const char *role    = NULL;
    const char *user    = NULL;
    const char *command = NULL;

    // parse --role, --user, and the command flag
    int i;
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--role") == 0 && i + 1 < argc) {
            role = argv[++i];
        } else if (strcmp(argv[i], "--user") == 0 && i + 1 < argc) {
            user = argv[++i];
        } else if (argv[i][0] == '-' && argv[i][1] == '-') {
            command = argv[i] + 2;
            i++;
            break;
        }
    }

    if (!role || !user || !command) {
        fprintf(stderr, "ERROR: --role, --user, and a command are required\n\n");
        usage(argv[0]);
        return 1;
    }

    if (strcmp(role, ROLE_MANAGER) != 0 && strcmp(role, ROLE_INSPECTOR) != 0) {
        fprintf(stderr, "ERROR: role must be 'manager' or 'inspector'\n");
        return 1;
    }

    char **args  = argv + i;
    int   n_args = argc - i;

    // add <district>
    if (strcmp(command, "add") == 0) {
        if (n_args < 1) { fprintf(stderr, "ERROR: --add requires <district>\n"); return 1; }
        const char *district = args[0];

        if (district_init(district) < 0) return 1;

        double lat, lon;
        char category[CATEGORY_LEN];
        int severity;
        char description[DESCRIPTION_LEN];

        printf("X: "); scanf("%lf", &lat);
        printf("Y: "); scanf("%lf", &lon);
        printf("Category (road/lighting/flooding/other): ");
        scanf("%31s", category);
        printf("Severity level (1/2/3): "); scanf("%d", &severity);
        printf("Description: ");
        getchar();
        fgets(description, sizeof(description), stdin);
        description[strcspn(description, "\n")] = '\0';

        if (report_add(district, user, lat, lon, category, severity, description) < 0)
            return 1;

        log_action(district, user, role, "add");
        return 0;
    }

    // list <district>
    if (strcmp(command, "list") == 0) {
        if (n_args < 1) { fprintf(stderr, "ERROR: --list requires <district>\n"); return 1; }
        const char *district = args[0];

        if (district_init(district) < 0) return 1;
        if (report_list(district) < 0) return 1;

        log_action(district, user, role, "list");
        return 0;
    }

    // view <district> <report_id>
    if (strcmp(command, "view") == 0) {
        if (n_args < 2) { fprintf(stderr, "ERROR: --view requires <district> <id>\n"); return 1; }
        const char *district = args[0];
        int report_id = atoi(args[1]);

        if (district_init(district) < 0) return 1;
        if (report_view(district, report_id) < 0) return 1;

        log_action(district, user, role, "view");
        return 0;
    }

    // remove_report <district> <report_id> — manager only
    if (strcmp(command, "remove_report") == 0) {
        if (strcmp(role, ROLE_MANAGER) != 0) {
            fprintf(stderr, "ERROR: remove_report is restricted to managers\n");
            return 1;
        }
        if (n_args < 2) { fprintf(stderr, "ERROR: --remove_report requires <district> <id>\n"); return 1; }
        const char *district = args[0];
        int report_id = atoi(args[1]);

        if (district_init(district) < 0) return 1;
        if (report_remove(district, report_id) < 0) return 1;

        log_action(district, user, role, "remove_report");
        return 0;
    }

    // update_threshold <district> <value> — manager only
    if (strcmp(command, "update_threshold") == 0) {
        if (strcmp(role, ROLE_MANAGER) != 0) {
            fprintf(stderr, "ERROR: update_threshold is restricted to managers\n");
            return 1;
        }
        if (n_args < 2) { fprintf(stderr, "ERROR: --update_threshold requires <district> <value>\n"); return 1; }
        const char *district = args[0];
        int value = atoi(args[1]);

        if (district_init(district) < 0) return 1;

        char cfg_path[512];
        district_path(district, CONFIG_FILE, cfg_path, sizeof(cfg_path));
        if (perm_check(cfg_path, role, "write") < 0) return 1;

        if (district_set_threshold(district, value) < 0) return 1;
        printf("Threshold for district '%s' updated to %d\n", district, value);

        log_action(district, user, role, "update_threshold");
        return 0;
    }

    // filter <district> <cond> [cond...]
    if (strcmp(command, "filter") == 0) {
        if (n_args < 2) { fprintf(stderr, "ERROR: --filter requires <district> and at least one condition\n"); return 1; }
        const char *district = args[0];
        const char **conditions = (const char **)(args + 1);
        int n_conditions = n_args - 1;

        if (district_init(district) < 0) return 1;
        if (filter_reports(district, conditions, n_conditions) < 0) return 1;

        log_action(district, user, role, "filter");
        return 0;
    }

    fprintf(stderr, "ERROR: unknown command '%s'\n\n", command);
    usage(argv[0]);
    return 1;
}
