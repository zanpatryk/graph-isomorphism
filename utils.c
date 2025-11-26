#include "utils.h"
#include <string.h>
#include <ctype.h>

char *trim_inplace(char *s) {
    if (!s) return s;
    size_t len = strlen(s);
    while (len > 0 && (s[len - 1] == '\n' || s[len - 1] == '\r')) s[--len] = '\0';
    char *start = s;
    while (*start && isspace((unsigned char) *start)) start++;
    char *end = start + strlen(start);
    while (end > start && isspace((unsigned char) *(end - 1))) *(--end) = '\0';
    if (start != s) memmove(s, start, strlen(start) + 1);
    return s;
}
