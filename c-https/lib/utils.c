#include "utils.h"


// timestamp converts a unix timestamp to an RFC-3339 date string
char * timestamp(time_t t, bool to_utc) {
    struct tm * lt = malloc(sizeof(struct tm));
    if (to_utc)
        gmtime_r(&t, lt);
    else
        localtime_r(&t, lt);
    char res[32];
    if (strftime(res, sizeof(res), date_format, lt) == 0) {
        perror("strftime");
        exit(1);
    }
    free(lt);
    return strdup(res);
}

// str2time returns a time_t from a date time string
time_t str2time(const char * time_str) {
    struct tm lt;
    strptime(time_str, date_format, &lt);
    return mktime(&lt);
}

// starts_with_any checks if a starts with any character in b
bool starts_with_any(char * a, char * b) {
    for (int i = 0; i < strlen(b); i++)
        if (strncmp(a, &b[i], 1) == 0)
            return true;
    return false;
}