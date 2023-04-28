#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../lib/client_controller.h"
#include "../lib/pres_layer.h"
#include "../lib/command.h"
#include "../lib/common.h"

void test_get_cmd_len(char*);
void test_get_code(char*);
void test_strslice(char*, int, int);
void test_str_to_cmd(char*);
void test_cmd_to_str(struct command);
void test_is_valid_cmd(struct command);
void test_send_command_msg(char*, int);

void run_get_cmd_len_tests();
void run_get_code_tests();
void run_strslice_tests();
void run_str_to_cmd_tests();
void run_cmd_to_str_tests();
void run_is_valid_cmd_tests();
void run_read_command_str_tests();
void run_send_command_msg_tests();

int main(int argc, char** argv) {
    // run_get_cmd_len_tests();
    // run_get_code_tests();
    // run_strslice_tests();
    // run_str_to_cmd_tests();
    // run_cmd_to_str_tests();
    run_is_valid_cmd_tests();
    // run_read_command_str_tests();
    // run_send_command_msg_tests();
    

    return 0;
}

void test_get_cmd_len(char* str) {
    int testval = get_cmd_len(str, strlen(str));
    printf("get_cmd_len: %s yields %d\n", str, testval);
}

void run_get_cmd_len_tests() {
    printf("=============================================\n");

    char* get_cmd_len_tests[] = {
        "MOVD|12|blahblahblah",
        "MOVD|123|",
        "MOVD|12",
        "MOVD|1w2|amsbn",
        "MOVD|2|4|45|35|",
        "MOVD||"
    };

    int ntests = sizeof(get_cmd_len_tests)/sizeof(char*);
    for (int i=0; i<ntests; ++i) {
        test_get_cmd_len(get_cmd_len_tests[i]);
    }
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

void test_is_valid_cmd(struct command cmd) {
    char* err = malloc(1000);

    printf("\n********* is_valid_cmd input *********\n");
    display_cmd(cmd);
    printf("Result: %s\n", is_valid_cmd(cmd, err) ? "valid" : "invalid");
    if (strlen(err) > 0) {
        printf("Error message: %s\n", err);
    }
    free(err);
}

void run_is_valid_cmd_tests() {
    printf("=============================================\n");

    struct command tests[] = {
        {RSGN, 0, 0, 0},
        {RSGN, "A", 0, 0},
        {PLAY, 0, 0, 0},
        {PLAY, "Name1", 0, 0},
        {PLAY, "Name1", "Name2", 0},
        {DRAW, 0, 0, 0},
        {DRAW, "S", 0, 0},
        {DRAW, "X", 0, 0},
        {DRAW, "S", "A", 0},
        {MOVE, 0, 0, 0},
        {MOVE, "X", 0, 0},
        {MOVE, "X", "1,2", 0},
        {MOVE, "Y", "1,2", 0},
        {MOVE, "X", "1,0", 0},
        {MOVE, "X", "1,2", "O"},
    };

    int ntests = sizeof(tests)/sizeof(struct command);
    for (int i=0; i<ntests; ++i) {
        test_is_valid_cmd(tests[i]);
    }
}

void run_read_command_str_tests() {
    printf("=============================================\n");

    char* buf = malloc(4 * sizeof(char));
    char* leftovers = malloc(101 * sizeof(char));

    int res = read_command_str(STDIN_FILENO, buf, 3, leftovers);

    printf("******* Results of read_command_str *******\n");
    printf("   buf: '%s'\n", buf);
    printf("   leftovers: '%s'\n", leftovers);
    printf("   bytes written: %d\n", res);

    free(buf);
    free(leftovers);
}

void test_send_command_msg(char* msg, int bufsize) {
    printf("\nRunning send_command_msg on '%s':\n", msg);
    int ret = send_command_msg(STDOUT_FILENO, msg, bufsize);
    printf("\nReturn value: %d\n", ret);
}

void run_send_command_msg_tests() {
    printf("=============================================\n");
    int bufsize = 6;
    char* tests[] = {
        "WAIT|0|",
        "This is a very very very very very very very long message!"
    };
    
    
    int ntests = sizeof(tests)/sizeof(char*);
    for (int i=0; i<ntests; ++i) {
        test_send_command_msg(tests[i], bufsize);
    }
}