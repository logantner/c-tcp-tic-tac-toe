#ifndef COMMAND_H_
#define COMMAND_H_

typedef enum {
    PLAY = 0,
    MOVE = 1,
    RSGN = 2,
    DRAW = 3,
    WAIT = 4,
    BEGN = 5,
    MOVD = 6,
    INVL = 7,
    OVER = 8,
    BADV = 9,  // placeholder for invalid command codes
} cmd_code;

struct command {
    cmd_code code;
    char* arg1;
    char* arg2;
    char* arg3;
};

void free_cmd(struct command);

// command/code conversions
int str_to_cmd(char*, int, struct command*, char*);
cmd_code str_to_code(char*);
char* cmd_to_str(struct command);
char* code_to_str(cmd_code);
cmd_code get_code(char*);

// command-building functions
struct command new_cmd();
// server commands
struct command new_wait_cmd();
struct command new_begn_cmd(char*);
struct command new_movd_cmd(char*, int, int, char*);
struct command new_invl_cmd(char*);
struct command new_draw_cmd(char*);
struct command new_over_cmd(char*, char*);
// client commands
struct command new_play_cmd(char*);
struct command new_move_cmd(char*, int, int);
struct command new_rsgn_cmd();

#endif