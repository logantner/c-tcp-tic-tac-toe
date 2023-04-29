#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common_tests.h"
#include "../common.h"

void test_strslice(char*, int, int);

void run_all_common_tests() {
    //// run_is_number_tests()
    run_strslice_tests();
}

void test_strslice(char* teststr, int start, int end) {
    char* result = strslice(teststr, start, end);
    printf("strslice(%s, %d, %d) yields '%s'\n", teststr, start, end, result);
    free(result);
}

void run_strslice_tests() {
    printf("=============================================\n");

    test_strslice("Testing string slice", 3, 8);
    test_strslice("Testing string slice", 0, 8);
    char* teststr = "Example";
    test_strslice(teststr, 2, strlen(teststr));
    test_strslice(teststr, 2, strlen(teststr)+1);
}