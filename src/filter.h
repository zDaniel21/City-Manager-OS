#ifndef FILTER_H
#define FILTER_H

#include "report.h"

// splits "field:operator:value" into 3 parts
int parse_condition(const char *input, char *field, char *op, char *value);

// returns 1 if report matches the condition, 0 otherwise
int match_condition(const Report *r, const char *field,
                    const char *op, const char *value);

// prints all reports from district matching all conditions
int filter_reports(const char *district, const char **conditions, int n_conditions);

#endif 