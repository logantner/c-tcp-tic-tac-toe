#ifndef PRES_LAYER_H
#define PRES_LAYER_H

#include "command.h"

typedef enum {
    TRANS_OK,           // Read or send was successful
    READ_CONN_CLOSED,   // Read failed because the connection was closed
    READ_CONN_FAILED,   // Read failed due to unknown connection issue
    READ_INVL_MSG,      // Read succeeded but the message could not be parsed
    SEND_FAILED         // Attempt to send message failed
} trans_code;

// transmission functions
int read_command_str(int, char*, int, char*);
trans_code read_command(int, struct command*, char*, char*);
int send_command_msg(int, char*, int);
int send_command(int, struct command);

// other command/code functions
int get_cmd_len(char*, int);
void display_cmd(struct command);
int is_valid_cmd(struct command);

#endif