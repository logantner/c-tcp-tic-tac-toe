#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "common.h"

// Returns true if str is nonempty and consists entirely of digit chars
int is_number(const char* const str) {
    if (str == NULL || strlen(str) == 0) {
        return 0;
    }

    for (int i=0; i<strlen(str); ++i) {
        if (!isdigit(str[i])) {
            return 0;
        }
    }
    return 1;
}

// Returns a newly allocated string containing characters from fromstr in the
// range [start, end). User is responsible for freeing returned value.
char* strslice(char* fromstr, int start, int end) {
    int maxlen = strlen(fromstr);
    if (start >= maxlen || end > maxlen || start < 0 || end < 0 || start > end) {
        return NULL;
    }

    char* lenstr = malloc((end - start + 1) * sizeof(char));
    memcpy(lenstr, fromstr+start, end-start);
    return lenstr;
}