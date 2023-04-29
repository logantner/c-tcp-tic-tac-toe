#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "presentation_tests.h"
#include "../presentation.h"
#include "../config.h"

void test_send_command_msg(char*, int);
void test_send_command(int, struct command);
void test_get_cmd_len(char*);
void test_is_valid_cmd(struct command);


void run_all_presentation_tests() {
    // run_read_command_str_tests();   // requires user input
    // run_read_command_tests();       // requires user input
    run_send_command_msg_tests();
    run_send_command_tests();

    run_get_cmd_len_tests();
    run_is_valid_cmd_tests();
}


void run_read_command_str_tests() {
    printf("================================================\n");
    printf("============ READ_COMMAND_STR TESTS ============\n");
    printf("================================================\n");

    int bufsize = 20;
    char* buf = malloc((bufsize+1) * sizeof(char));
    char* leftovers = malloc(101 * sizeof(char));

    int res = read_command_str(STDIN_FILENO, buf, bufsize, leftovers);

    printf("******* Results of read_command_str *******\n");
    printf("   buf: '%s'\n", buf);
    printf("   leftovers: '%s'\n", leftovers);
    printf("   bytes written: %d\n", res);

    free(buf);
    free(leftovers);
}

void run_read_command_tests() {
    printf("============================================\n");
    printf("============ READ_COMMAND TESTS ============\n");
    printf("============================================\n");

    struct command cmd = new_cmd();

    char leftovers[MAX_DATA_PACKET_SIZE];
    // strcpy(leftovers, "PLAY|6|Logan|BLAH|0|");
    char err_msg[MAX_DATA_PACKET_SIZE];
    trans_code tcode = read_command(STDIN_FILENO, &cmd, leftovers, err_msg);

    display_cmd(cmd);
    printf("==> Returned trans code: %d (%s)\n", tcode, trans_code_to_str(tcode));
    printf("==> Leftover storage: '%s'\n", leftovers);
    printf("==> Error message: '%s'\n", err_msg);

    // tcode = read_command(STDIN_FILENO, &cmd, leftovers, err_msg);

    // display_cmd(cmd);
    // printf("==> Returned trans code: %d (%s)\n", tcode, trans_code_to_str(tcode));
    // printf("==> Leftover storage: '%s'\n", leftovers);
    // printf("==> Error message: '%s'\n", err_msg);

    free_cmd(cmd);
}

void run_send_command_tests() {
    printf("============================================\n");
    printf("============ SEND_COMMAND TESTS ============\n");
    printf("============================================\n");

    struct command tests[] = {
        {WAIT, 0, 0, 0},
        {DRAW, "A", 0, 0},
        {BEGN, "X", "Logan Gantner", 0},
        {MOVD, "O", "1,1", "...X.O..X"}
    };

    int fake_fd = 10;

    int ntests = sizeof(tests)/sizeof(struct command);
    for (int i=0; i<ntests; ++i) {
        // Tests to stdout
        test_send_command(STDOUT_FILENO, tests[i]);
    }
    // Test to invalid/closed fd
    test_send_command(fake_fd, tests[0]);
}

void test_send_command(int fd, struct command cmd) {
    trans_code tcode = send_command(fd, cmd);
    if (fd == STDOUT_FILENO) { printf("\n"); }
    if (tcode != TRANS_OK) {
        printf("==> Returned trans code: %d (%s)\n", tcode, trans_code_to_str(tcode));
    }
}

void run_send_command_msg_tests() {
    printf("================================================\n");
    printf("============ SEND_COMMAND_MSG TESTS ============\n");
    printf("================================================\n");
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

void test_send_command_msg(char* msg, int bufsize) {
    printf("\nRunning send_command_msg on '%s':\n", msg);
    int ret = send_command_msg(STDOUT_FILENO, msg, bufsize);
    printf("\nReturn value: %d\n", ret);
}

void run_is_valid_cmd_tests() {
    printf("============================================\n");
    printf("============ IS_VALID_CMD TESTS ============\n");
    printf("============================================\n");

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

void run_get_cmd_len_tests() {
    printf("===========================================\n");
    printf("============ GET_CMD_LEN TESTS ============\n");
    printf("===========================================\n");

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

void test_get_cmd_len(char* str) {
    int testval = get_cmd_len(str, strlen(str));
    printf("get_cmd_len: %s yields %d\n", str, testval);
}