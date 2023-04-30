#ifndef PRESENTATION_H
#define PRESENTATION_H

#include "command.h"

typedef enum {
    TRANS_OK = 0,           // Read or send was successful, no detected issues
    READ_OK_INVL_CMD,   // Read succeeded and message parsed, but command is structurally not valid
                        //    - Unrecognized command name, unexpected arg values, etc
    READ_CONN_CLOSED,   // Read failed because the connection was closed
    READ_CONN_FAILED,   // Read failed due to unknown connection issue
    READ_INVL_MSG,      // Read succeeded but the message could not be parsed
    SEND_FAILED         // Attempt to send message failed
} trans_code;

// transmission functions
int read_command_str(int, char*, int, char*);
trans_code read_command(int, struct command*, char*, char*);
int send_command_msg(int, char*, int);
trans_code send_command(int, struct command);
char* trans_code_to_str(trans_code);

// other command/code functions
int get_cmd_len(char*, int);
int is_valid_cmd(struct command, char*);

#endif