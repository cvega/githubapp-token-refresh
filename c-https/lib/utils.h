#ifndef UTILS_H
#define UTILS_H

#define _XOPEN_SOURCE
#define _GNU_SOURCE

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static const char date_format[] = "%Y-%m-%dT%H:%M:%SZ";

char * timestamp(time_t, bool);
time_t str2time(const char *);
bool starts_with_any(char *, char *);

#endif