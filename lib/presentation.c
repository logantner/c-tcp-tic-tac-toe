#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "command.h"
#include "presentation.h"
#include "common.h"
#include "config.h"



// Command validation tools
int has_n_args(struct command, int, char*);
int is_draw_type(char*, char*);
int is_role(char*, char*);
int is_cell(char*, char*);

// Miscellaneous helper functions
int get_pre_len(char*);
int read_msg_part(int, char**, int*, int*);


// Parses and returns the command length from unparsed message string.
// Returns -1 if the msg is too short to be parsed yet, or -2 if the msg is improperly formatted
int get_cmd_len(char* msg, int msg_len) {
    int ret;
    int start = -1, end = -1;
    for (int i=0; i < msg_len; ++i) {
        if (msg[i] == '|') {
            if (start < 0) {
                start = i;
            } else {
                end = i;
                break;
            }
        } 
    }

    if (start >= 0 && end > 0) {
        char* lenstr = strslice(msg, start+1, end);

        if (is_number(lenstr)) {
            ret = atoi(lenstr);
        } else {
            ret = -2;
        }
        free(lenstr);

    } else {
        ret = -1;
    }

    return ret;
}

cmd_code get_code(char* msg) {
    if (strlen(msg) < 4 || msg[4] != '|') {
        return BADV;
    }

    char code[5];
    memcpy(code, msg, 4);
    code[4] = 0;

    return str_to_code(code);
}

cmd_code str_to_code(char* codestr) {
    if (strcmp(codestr, "PLAY") == 0) {
        return PLAY;
    } else if (strcmp(codestr, "MOVE") == 0) {
        return MOVE;
    } else if (strcmp(codestr, "RSGN") == 0) {
        return RSGN;
    } else if (strcmp(codestr, "DRAW") == 0) {
        return DRAW;
    } else if (strcmp(codestr, "WAIT") == 0) {
        return WAIT;
    } else if (strcmp(codestr, "BEGN") == 0) {
        return BEGN;
    } else if (strcmp(codestr, "MOVD") == 0) {
        return MOVD;
    } else if (strcmp(codestr, "INVL") == 0) {
        return INVL;
    } else if (strcmp(codestr, "OVER") == 0) {
        return OVER;
    }

    return BADV;
}

void display_cmd(struct command cmd) {
    printf("Command code: %s\n", code_to_str(cmd.code));
    printf("   Arg 1: %s\n", cmd.arg1);
    printf("   Arg 2: %s\n", cmd.arg2);
    printf("   Arg 3: %s\n", cmd.arg3);
}

// Returns true if arguments of provided command are valid based on its code value.
// Validating commands is only intended to be used by the server, and only accepts
// legal client calls as a result--server commands always return false.
// Stores any relevent feedback message to err.
int is_valid_cmd(struct command cmd, char* err) {
    memset(err, 0, strlen(err));
    switch(cmd.code) {
        case PLAY:
            return has_n_args(cmd, 1, err);
        case MOVE:
            return has_n_args(cmd, 2, err) && is_role(cmd.arg1, err) && is_cell(cmd.arg2, err);
        case RSGN:
            return has_n_args(cmd, 0, err);
        case DRAW:
            return has_n_args(cmd, 1, err) && is_draw_type(cmd.arg1, err);
        // case BEGN:
        // case MOVD:
        // case INVL:
        // case OVER:
        // case WAIT:
        // case BADV:
        default:
            return 0;
    }
}

// Returns true if cmd has precisely n arguments
int has_n_args(struct command cmd, int n, char* err_msg) {
    char* args[3] = {cmd.arg1, cmd.arg2, cmd.arg3};

    if (n < 0 || n > 3) {
        return 0;
    }

    for (int i=0; i<3; ++i) {
        // First n must not be NULL
        if (args[i] == NULL && i < n) {
            sprintf(err_msg, "%s must have precisely %d args", code_to_str(cmd.code), n);
            return 0;
        }
        // Remaining 3-n must be NULL
        if (args[i] != NULL && i >= n) {
            sprintf(err_msg, "%s must have precisely %d args", code_to_str(cmd.code), n);
            return 0;
        }
    }

    return 1;
}

int is_draw_type(char* arg, char* err_msg) {
    int ret = 1;

    if (arg == NULL || strlen(arg) != 1) {
        ret = 0;
    } else if (arg[0] != 'S' && arg[0] != 'A' && arg[0] != 'R') {
        ret = 0;
    }

    if (ret == 0) {
        sprintf(err_msg, "'%s' is not a valid type of draw response. Choose 'S', 'R' or 'A'", arg);
    }

    return ret;
}

int is_role(char* arg, char* err_msg) {
    int ret = 1;
    memcpy(err_msg, 0, strlen(err_msg));
    if (arg == NULL || strlen(arg) != 1) {
        ret = 0;
    } else if (arg[0] != 'X' && arg[0] != 'O') {
        ret = 0;
    }
    
    if (ret == 0) {
        sprintf(err_msg, "'%s' us not a valid player role. Choose 'X' or 'O'", arg);
    }
    return ret;
}

int is_cell(char* arg, char* err_msg) {
    int ret = 1;
    memcpy(err_msg, 0, strlen(err_msg));
    // Must be exactly 3 characters
    if (arg == NULL || strlen(arg) != 3) {
        ret = 0;
    }
    // First char must be 1, 2 or 3
    else if (arg[0] != '1' && arg[0] != '2' && arg[0] != '3') {
        ret = 0;
    }
    // Second char must be a comma
    else if (arg[1] != ',') {
        ret = 0;
    }
    // Third char must be 1, 2 or 3
    else if (arg[2] != '1' && arg[2] != '2' && arg[2] != '3') {
        ret = 0;
    }

    if (ret == 0) {
        sprintf(err_msg, "'%s' is not a valid cell choice. Must be in format 'r,c', with r,c in {1,2,3}", arg);
    }
    return 1;
}

// Continually reads command message from fd and stores into buf, expanding buf as necessary.
// Because buf may be realloc'd as needed, user must provide a malloc'd or calloc'd buf in advance.
// If the fd stream spills over into a second message, the start of the second message will be 
// stored within the leftovers buffer.
// Returns the number of bytes written to buf, 0 if connection was closed, or -1 if fd stream data could not be parsed.
int read_command_str(int fd, char* buf, int bufsize, char* leftovers) {
    
    int bytes_read;
    int totlen = strlen(buf);                // the current length of the string stored in buf
    int argslen = get_cmd_len(buf, totlen);  // the args length parsed from the 2nd field of the message
    int explen;                              // the expected length once the message is totally read

    // keep reading until we are able to parse the args length
    while (argslen == -1) {
        bytes_read = read_msg_part(fd, &buf, &bufsize, &totlen);

        if (bytes_read <= 0) {
            // The connection has closed or failed: abort
            return bytes_read;
        }
        argslen = get_cmd_len(buf, totlen);
    }

    // if args length was not included or is improperly formatted, abort
    if (argslen == -2) {
        return -1;
    }
    // We are now aware of how long the message is expected to be
    explen = get_pre_len(buf) + argslen;

    // keep reading until msg length equals or exceeds expected length
    while (totlen < explen) {
        bytes_read = read_msg_part(fd, &buf, &bufsize, &totlen);
        if (bytes_read <= 0) {
            // The connection has closed or failed: abort
            return bytes_read;
        }
    }

    // if there was spillover, store the extra data in leftovers and clear it from buf
    if (totlen > explen) {
        strcpy(leftovers, buf + explen);
        memset(buf + explen, 0, bufsize - explen);
        totlen -= strlen(leftovers);
    }

    return totlen;
}

// Attempts to read and parse a command from fd, storing the result into cmd and storing any
// runoff command string into leftovers. If there are leftovers from a previous read, they must
// be contained in the leftovers data.
// Returns a read code indicating the type of issue encountered.
// In the event that READ_INVL_MSG is returned, a more specific error message is saved to errmsg
//
// TIP: free_cmd(cmd) should be called exactly once for each time read_command(...) is called
trans_code read_command(int fd, struct command* cmd, char* leftovers, char* errmsg) {

    // copy starting_str into sufficiently large buffer
    char* cmdstr = strdup(leftovers);
    int bufsize = strlen(cmdstr);
    if (bufsize < MAX_DATA_PACKET_SIZE) {
        cmdstr = realloc(cmdstr, MAX_DATA_PACKET_SIZE);
        bufsize = MAX_DATA_PACKET_SIZE;
    }

    // clean out leftovers and errmsg for filling with fresh data
    memset(leftovers, 0, strlen(leftovers));
    memset(errmsg, 0, strlen(errmsg));

    int bytes_read = read_command_str(fd, cmdstr, bufsize, leftovers);
    if (bytes_read <= 0) {
        free(cmdstr);
        return bytes_read == 0 ? READ_CONN_CLOSED : READ_CONN_FAILED;
    }

    if( str_to_cmd(cmdstr, strlen(cmdstr), cmd, errmsg) ) {
        // String was read but cannot be parsed as a command.
        free(cmdstr);
        return READ_INVL_MSG;
    }

    // Read was successful and command has been stored in cmd.
    free(cmdstr);
    return TRANS_OK;
}

// Reads whatever message part is available from fd stream into buf, realloc'ing and updating 
// the buffer size and string length as needed
int read_msg_part(int fd, char** buf, int* bufsize, int* curlen) {
    int totlen = *curlen;

    // expand buf if needed
    if (totlen == *bufsize) {
        *bufsize *= 2;
        *buf = realloc(*buf, (*bufsize + 1) * sizeof(char));
    }

    int bytes_read = read(fd, *buf+totlen, *bufsize-totlen);

    // remove linebreak when testing using stdin. Can ignore otherwise
    if (fd == STDIN_FILENO && bytes_read > 0 && (*buf)[totlen + bytes_read - 1] == '\n') {
        (*buf)[totlen + bytes_read - 1] = 0;
        --bytes_read;
    }

    // only update the string length if any bytes were written
    if (bytes_read > 0) {
        *curlen += bytes_read;
    }

    return bytes_read;
}

// Attempts to send command message to specified file descriptor. Returns -1 if this fails, 0 otherwise.
// Message may be sent in pieces, depending on max_packet_size
int send_command_msg(int fd, char* cmdstr, int max_packet_size) {

    int bytes_to_send = strlen(cmdstr);
    int tot_bytes_sent = 0;
    char buf[max_packet_size];

    while (tot_bytes_sent < bytes_to_send) {
        int rem_bytes = bytes_to_send - tot_bytes_sent;
        int packet_size = (rem_bytes > max_packet_size - 1) ? (max_packet_size - 1) : rem_bytes;

        int bytes_sent = write(fd, cmdstr + tot_bytes_sent, packet_size);
        if (bytes_sent <= 0) {
            fprintf(stderr, "Failed to send message to fd %d\n", fd);
            return -1;
        }

        tot_bytes_sent += bytes_sent;
    }

    return 0;
}

// Attempts to send command in message format to specified file descriptor. 
// Returns SEND_FAILED if this fails, TRANS_OK otherwise.
trans_code send_command(int client_fd, struct command cmd) {
    char* cmdstr = cmd_to_str(cmd);
    int send_code = send_command_msg(client_fd, cmdstr, MAX_DATA_PACKET_SIZE);
    free(cmdstr);
    return send_code ? SEND_FAILED : TRANS_OK;
}

char* trans_code_to_str(trans_code tcode) {
    switch(tcode) {
        case TRANS_OK:
            return "TRANS_OK";
        case READ_OK_INVL_CMD:
            return "READ_OK_INVL_CMD";
        case READ_CONN_CLOSED:
            return "READ_CONN_CLOSED";
        case READ_CONN_FAILED:
            return "READ_CONN_FAILED";
        case READ_INVL_MSG:
            return "READ_INVL_MSG";
        case SEND_FAILED:
            return "SEND_FAILED";
    }
}