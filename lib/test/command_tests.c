#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "command_tests.h"
#include "../command.h"
#include "../presentation.h"

void test_get_code(char*);
void test_str_to_cmd(char*);
void test_cmd_to_str(struct command);

void run_all_command_tests() {
    run_str_to_cmd_tests();
    // run_str_to_code_tests();
    run_cmd_to_str_tests();
    // run_code_to_str_tests();
    run_get_code_tests();

    //// run_new_command_tets()
    //// ...
}

void test_get_code(char* msg) {
    int testval = get_code(msg);
    char* testval2 = code_to_str(testval);
    printf("get_code: %s yields %d (%s)\n", msg, testval, testval2);
}

void run_get_code_tests() {
    printf("=============================================\n");

    char* get_code_tests[] = {
        "MOVD|12|blahblahblah",
        "PLAY|X|1,1",
        "WAIT|0|",
        "MOVE|12|",
        "RSGN|124|",
        "DRAW|2|A|",
        "BEGN|2|X|",
        "INVL|12|Illegal move!|",
        "OVER|2|W|",
        "FAKE|12|whatever|",
        "DRA"
    };

    int ntests = sizeof(get_code_tests)/sizeof(char*);
    for (int i=0; i<ntests; ++i) {
        test_get_code(get_code_tests[i]);
    }
}

void test_str_to_cmd(char* teststr) {
    struct command cmd;
    char err[100];
    int res = str_to_cmd(teststr, strlen(teststr), &cmd, err);

    if (res == 0) {
        printf("str_to_cmd( %s ) succeeded in creating the following command:\n", teststr);
        printf("   ");
        display_cmd(cmd);
    } else {
        printf("str_to_cmd( %s ) failed with the following error message:\n", teststr);
        printf("   %s\n", err);
    }

    free_cmd(cmd);
}

void run_str_to_cmd_tests() {
    printf("=============================================\n");

    char* str_to_cmd_tests[] = {
        // Valid tests
        "WAIT|0|",
        "WAIT|0||",
        "INVL|22|The command entered was not valid|",
        "MOVE|10|O|2,0|",
        "MOVD|16|X|1,1|........X|",
        // Invalid tests
        "MOVD|16|X|1,1|........X",
        "WHAT|4|ARG1|",
        "MOVD|16|X|1,1|........X|EXTRA_ARG|",
        "WAT|0|"
    };

    int ntests = sizeof(str_to_cmd_tests)/sizeof(char*);
    for (int i=0; i<ntests; ++i) {
        test_str_to_cmd(str_to_cmd_tests[i]);
    }
}

void test_cmd_to_str(struct command cmd) {
    char* cmdstr = cmd_to_str(cmd);
    printf("\n********* cmd_to_str input *********\n");
    display_cmd(cmd);
    printf("Result: '%s'\n", cmdstr);
    free(cmdstr);
}

void run_cmd_to_str_tests() {
    printf("=============================================\n");

    struct command cmd_to_str_tests[] = {
        {WAIT, 0, 0, 0},
        {DRAW, "A", 0, 0},
        {BEGN, "X", "Logan Gantner", 0},
        {MOVD, "O", "1,1", "...X.O..X"}
    };

    int ntests = sizeof(cmd_to_str_tests)/sizeof(struct command);
    for (int i=0; i<ntests; ++i) {
        test_cmd_to_str(cmd_to_str_tests[i]);
    }
}