#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "command.h"
#include "common.h"

char* const cmd_code_names[9] = {
    "PLAY",
    "MOVE",
    "RSGN",
    "DRAW",
    "WAIT",
    "BEGN",
    "MOVD",
    "INVL",
    "OVER"
};


// Returns the number of characters in msg prior to any args
int get_pre_len(char* msg) {
    int bar_count = 0;
    int i = 0;
    while (bar_count < 2) {
        if (i > strlen(msg)) {
            return -1;
        }

        if (msg[i] == '|') {
            ++bar_count;
        }
        ++i;
    }
    return i;
}

// Converts given string cmdstr to a command struct, stored in cmd.
// Returns -1 if the string cannot be interpretted as a valid command, 0 otherwise.
// If -1 is returned, explanatory error message is stored in errmsg.
// Args must be freed by the user, and can easily be freed using free_cmd(...)
int str_to_cmd(char* cmdstr, int strlen, struct command* cmd, char* errmsg) {

    // clear out the contents of the command
    memset(cmd, 0, sizeof(struct command));

    // get and validate command code
    cmd_code code = get_code(cmdstr);
    if (code == BADV) {
        strcpy(errmsg, "Command code is not valid");
        return -1;
    }
    cmd->code = code;

    // get and store a maximum of three arguments
    int bar_count = 1;
    int start, end;
    for (int i=5; i<strlen; ++i) {
        if (cmdstr[i] == '|') {
            ++bar_count;
            end = i;
            if (bar_count == 3) {
                cmd->arg1 = strslice(cmdstr, start+1, end);
            } else if (bar_count == 4) {
                cmd->arg2 = strslice(cmdstr, start+1, end);
            } else if (bar_count == 5) {
                cmd->arg3 = strslice(cmdstr, start+1, end);
                if (i != strlen - 1) {
                    strcpy(errmsg, "Command string contains an invalid number of args");
                    return -1;
                }
            }
            start = i;
        }
    }

    if (cmdstr[strlen-1] != '|') {
        strcpy(errmsg, "Final char of a command string must be a '|'");
        return -1;
    }

    return 0;
}

// Converts a struct command to a string that can be interpreted by servers or clients.
// The user is responsible for freeing the returned string.
char* cmd_to_str(struct command cmd) {

    // Figure out how long this string is going to be
    int argslen = 0;
    argslen += (cmd.arg1 == NULL ? 0 : strlen(cmd.arg1) + 1);
    argslen += (cmd.arg2 == NULL ? 0 : strlen(cmd.arg2) + 1);
    argslen += (cmd.arg3 == NULL ? 0 : strlen(cmd.arg3) + 1);

    // Get the args length as a string (the 2nd field)
    char argslenstr[10];
    sprintf(argslenstr, "%d", argslen);

    int totlen = 6 + strlen(argslenstr) + argslen;
    char* cmdstr = malloc((totlen + 1) * sizeof(char));

    // Add the command code (the 1st field)
    strcpy(cmdstr, code_to_str(cmd.code));
    cmdstr[4] = '|';

    // Add the args length (the 2nd field)
    strcpy(cmdstr + 5, argslenstr);
    cmdstr[strlen(cmdstr)] = '|';

    // Add the optional args, if any (the 3rd, 4th, and 5th fields)
    if (cmd.arg1 != NULL) {
        strcpy(cmdstr + strlen(cmdstr), cmd.arg1);
        cmdstr[strlen(cmdstr)] = '|';
    }

    if (cmd.arg2 != NULL) {
        strcpy(cmdstr + strlen(cmdstr), cmd.arg2);
        cmdstr[strlen(cmdstr)] = '|';
    }

    if (cmd.arg3 != NULL) {
        strcpy(cmdstr + strlen(cmdstr), cmd.arg3);
        cmdstr[strlen(cmdstr)] = '|';
    }

    return cmdstr;
}

char* code_to_str(cmd_code code) {
    char ret[5];
    if (code < 0 || code > 8) {
        return NULL;
    }

    return cmd_code_names[code];
}

// Frees the underlying command data. WARNING: This should only ever be used when a command was
// generated using the str_to_cmd method, or if all args are known to be malloc'd data. If you set 
// or modify arg values after the fact, you are responsible for freeing data as needed manually.
void free_cmd(struct command cmd) {
    if (cmd.arg1 != NULL) {
        free(cmd.arg1);
    }
    if (cmd.arg2 != NULL) {
        free(cmd.arg2);
    }
    if (cmd.arg3 != NULL) {
        free(cmd.arg3);
    }
}

// returns a struct command will nulled fields
struct command new_cmd() {
    struct command cmd;
    memset(&cmd, 0, sizeof(struct command));
    return cmd;
}

struct command new_wait_cmd() {
    struct command cmd = new_cmd();
    cmd.code = WAIT;
    return cmd;
}

struct command new_begn_cmd(char* role) {
    struct command cmd = new_cmd();
    cmd.code = BEGN;
    cmd.arg1 = role;
    return cmd;
}

struct command new_movd_cmd(char* role, int x, int y, char* grid) {
    char coords[4];
    sprintf(coords, "%d,%d", x, y);

    struct command cmd = new_cmd();
    cmd.code = MOVD;
    cmd.arg1 = role;
    cmd.arg1 = coords;
    cmd.arg1 = grid;
    return cmd;
}

struct command new_invl_cmd(char* reason) {
    struct command cmd = new_cmd();
    cmd.code = INVL;
    cmd.arg1 = reason;
    return cmd;
}

struct command new_draw_cmd(char* draw_req) {
    struct command cmd = new_cmd();
    cmd.code = DRAW;
    cmd.arg1 = draw_req;
    return cmd;
}

struct command new_over_cmd(char* win_type, char* outcome) {
    struct command cmd = new_cmd();
    cmd.code = OVER;
    cmd.arg1 = win_type;
    cmd.arg2 = outcome;
    return cmd;
}

struct command new_play_cmd(char* name) {
    struct command cmd = new_cmd();
    cmd.code = PLAY;
    cmd.arg1 = name;
    return cmd;
}

struct command new_move_cmd(char* role, int x, int y) {
    char coords[4];
    sprintf(coords, "%d,%d", x, y);

    struct command cmd = new_cmd();
    cmd.code = MOVE;
    cmd.arg1 = role;
    cmd.arg2 = coords;
    return cmd;
}

struct command new_rsgn_cmd() {
    struct command cmd = new_cmd();
    cmd.code = RSGN;
    return cmd;
}