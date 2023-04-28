#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "server_application.h"
#include "pres_layer.h"
#include "command.h"
#include "config.h"
#include "game.h"

trans_code get_player_response(struct command*, struct player*, struct ttt_game);
void end_game(struct ttt_game, int);
void toggle_player(struct player*, struct ttt_game);

trans_code process_resignation(struct command, struct ttt_game*, struct player);
trans_code process_move(struct command, struct ttt_game*, struct player*);
trans_code process_draw_req(struct command, struct ttt_game*, struct player*);
trans_code process_bad_context(struct command, struct player);
void handle_trans_failure(int, struct ttt_game, struct player);

// void set_up_game(int client_fd1, int client_fd2) {

// }

// Send BEGN to both players
// Wait to receive response from current player: <----------------------------------------------+
// - MOVE                                                                                       |
//    - If move is illegal, send INVL and wait for another response --------------------------->|
//    - If move is legal, apply it to the board and check to see if there is a win condition.   |
//        - If someone won or tied, send OVER to both and end game                              |
//        - If the game is not over, send MOVD to both and toggle player. Continue ------------>|
// - RSGN                                                                                       |
//    - Send OVER to both players and end the game                                              |
// - DRAW (Start a draw cycle)                                                                  |
//    - Toggle player. Forward DRAW S to other player                                           |
//    - Get DRAW response from other player.                                                    |
//        - If R, toggle player and forward DRAW R. Continue ---------------------------------->|
//        - If A, send OVER to both players. End game                                           |
// - Any other command                                                                          |                                  
//    - send INVL and continue ---------------------------------------------------------------->|
trans_code moderate_game(struct ttt_game game) {
    trans_code tcode = TRANS_OK;
    struct command in_cmd;
    struct player cur_p = game.p1;

    // Start game
    if (  send_command(game.p1.fd, new_begn_cmd("X")) == SEND_FAILED || 
          send_command(game.p2.fd, new_begn_cmd("O")) == SEND_FAILED ) {
        handle_trans_failure(SEND_FAILED, game, cur_p);
        return SEND_FAILED;
    }

    while (game.is_active) {
        tcode = get_player_response(&in_cmd, &cur_p, game);
        if (tcode != TRANS_OK) {
            free_cmd(in_cmd);
            continue;
        }
        
        if (in_cmd.code == MOVE) {
            tcode = process_move(in_cmd, &game, &cur_p);
        } else if (in_cmd.code == RSGN) {
            tcode = process_resignation(in_cmd, &game, cur_p);
        } else if (in_cmd.code == DRAW) {
            tcode = process_draw_req(in_cmd, &game, &cur_p);
        } else {
            tcode = process_bad_context(in_cmd, cur_p);
        }
        
        if (tcode != TRANS_OK && tcode != READ_OK_INVL_CMD) {
            handle_trans_failure(tcode, game, cur_p);
            game.is_active = 0;
        }

        free_cmd(in_cmd);
    }

    return tcode;
}

trans_code process_move(struct command move_cmd, struct ttt_game* game, struct player* cur_p) {
    char role = move_cmd.arg1[0];
    int row = move_cmd.arg2[0] - '0';
    int col = move_cmd.arg2[2] - '0';

    char space = get_board_val(*game, row, col);
    if (space == '.') {
        // This is a legal move: update the board, check for win conditions, then notify the players
        set_board_val(*game, row, col, role);
        char* win_status = get_win_status(*game);

        if (win_status == NULL) {
            // No winner or ties; game proceeds to next turn
            trans_code tcode1 = send_command(game->p1.fd, new_movd_cmd(move_cmd.arg1, row, col, game->board));
            trans_code tcode2 = send_command(game->p2.fd, new_movd_cmd(move_cmd.arg1, row, col, game->board));
            if (tcode1 == SEND_FAILED || tcode2 == SEND_FAILED) {
                return SEND_FAILED;
            }
            toggle_player(cur_p, *game);
            return TRANS_OK;
        }

        // The game needs to be concluded in some way
        if (strcmp(win_status, "T") == 0) {
            send_command(game->p1.fd, new_over_cmd("D", "The board is full with no winners"));
            send_command(game->p2.fd, new_over_cmd("D", "The board is full with no winners"));
        } else {
            send_command(cur_p->fd,   new_over_cmd("W", "You got three in a row!"));
            send_command(game->p2.fd, new_over_cmd("L", "Your opponent gor three in a row :("));
        }

        game->is_active = 0;
        return TRANS_OK;
        
    } else {
        // The player attempted to play in an occupied cell. Complain, and do not toggle the player
        return send_command(cur_p->fd, new_invl_cmd("Attempted to play in occupied cell"));
    }
}

trans_code process_resignation(struct command rsgn_cmd, struct ttt_game* game, struct player cur_p) {
    send_command(cur_p.fd, new_over_cmd("L", "You have resigned"));
    toggle_player(&cur_p, *game);
    send_command(cur_p.fd, new_over_cmd("W", "Your opponent has resigned"));
    game->is_active = 0;
    return TRANS_OK;
}

trans_code process_draw_req(struct command draw_cmd, struct ttt_game* game, struct player* cur_p) {
    if (strcmp(draw_cmd.arg1, "S") != 0) {
        // Only S is allowed for initiating a draw sequence. Complain to player, do not toggle
        return send_command(cur_p->fd, new_invl_cmd("No draw has been suggested yet. Send 'S' to suggest one"));
    }

    toggle_player(cur_p, *game);

    // forward draw suggestion to other player
    if (send_command(cur_p->fd, draw_cmd) == SEND_FAILED) {
        return SEND_FAILED;
    }

    // loop until players have completed a full draw cycle
    int tcode;
    while (1) {
        free_cmd(draw_cmd);
        tcode = get_player_response(&draw_cmd, cur_p, *game);

        if (tcode == READ_OK_INVL_CMD) {
            // handled by get_player_response() already - proceed to query player for new response
            continue;
        } else if (tcode != TRANS_OK) {
            // abort due to transmission failure
            return tcode;
        }

        // Allow player to resign, if this is something they want to do for some reason
        if (draw_cmd.code == RSGN) {
            return process_resignation(draw_cmd, game, *cur_p);
        }

        // Process draw response
        if (draw_cmd.code == DRAW) {
            if (strcmp(draw_cmd.arg1, "A") == 0) {
                // Draw is accepted - end the game
                send_command(game->p1.fd, new_over_cmd("D", "Both players gave agreed to a draw"));
                send_command(game->p2.fd, new_over_cmd("D", "Both players gave agreed to a draw"));
                game->is_active = 0;
                return TRANS_OK;
            } else if (strcmp(draw_cmd.arg1, "R") == 0) {
                // Draw is rejected - punt back to original player
                toggle_player(cur_p, *game);
                return send_command(cur_p->fd, draw_cmd);
            } else {
                // Complain that a new draw request cannot be made
                tcode = send_command(cur_p->fd, new_invl_cmd("A draw has already been suggested. Choose 'A' to accept or 'R' to reject"));
                if (tcode == SEND_FAILED) {
                    return SEND_FAILED;
                }
            }
        } else {
            // Complain that any other type of command is invalid right now
            char invl_resp[100];
            sprintf(invl_resp, "%s is not a valid response to a draw request", code_to_str(draw_cmd.code));
            if (send_command(cur_p->fd, new_invl_cmd(invl_resp)) == SEND_FAILED) {
                return SEND_FAILED;
            }
        }
    }
}

trans_code process_bad_context(struct command bad_cmd, struct player cur_p) {
    char bad_cmd_msg[50];
    sprintf(bad_cmd_msg, "%s is not a legal command at this moment", code_to_str(bad_cmd.code));
    return send_command(cur_p.fd, new_invl_cmd(bad_cmd_msg));
}

void handle_trans_failure(int tcode, struct ttt_game game, struct player cur_p) {
    toggle_player(&cur_p, game);

    if (tcode == READ_CONN_CLOSED) {
        send_command(cur_p.fd, new_over_cmd("W", "Your opponent has left the game"));
    } else if (tcode == READ_CONN_FAILED) {
        send_command(cur_p.fd, new_over_cmd("D", "Your opponent had connection issues. Let's call this one a draw"));
    } else if (tcode == READ_INVL_MSG) {
        // Do nothing - this was already handled in get_player_response
    }else if (tcode == SEND_FAILED) {
        // send failure may have affected either or both players. Attempt to update both and move on
        send_command(game.p1.fd, new_over_cmd("D", "There were connection issues. Let's call this one a draw"));
        send_command(game.p2.fd, new_over_cmd("D", "There were connection issues. Let's call this one a draw"));
    }
}

// void end_game(struct ttt_game game, int err_code) {
//     if (err_code < 0) {
//         // Do some extra stuff
//     }

//     close(game.p1.fd);
//     close(game.p2.fd);
// }

// Toggles player *p to point to their opponent
void toggle_player(struct player* p, struct ttt_game game) {
    if (p->fd == game.p1.fd) {
        *p = game.p2;
    } else {
        *p = game.p1;
    }
}

// Waits for input from the player, parses the message into a command struct, and stores the contents in in_cmd.
// Any read fragments from a previous read are supplied through p, which will be rewritten with any new
// read fragments. The contents of in_cmd should be freed using free_cmd to avoid any memory leaks.
//
// Any READ_INVL_MSG or READ_OK_INVL_CMD issues are handled immediately here
trans_code get_player_response(struct command* in_cmd, struct player* p, struct ttt_game game) {
    memset(in_cmd, 0, sizeof(struct command));
    char* err_msg = malloc(300 * sizeof(char));
    trans_code tcode = read_command(p->fd, in_cmd, p->msg_fragment, err_msg);

    if (tcode == READ_INVL_MSG) {
        // This is a presentation error. We cannot safely read any more messages from the player
        char invl_msg[1000];
        sprintf(invl_msg, "Your message could not even be parsed: '%s'\nEnding game and closing connection", err_msg);
        send_command(p->fd, new_invl_cmd(invl_msg));

        toggle_player(p, game);
        send_command(p->fd, new_over_cmd("D", "Your opponent had a connection issue. Let's call this one a draw."));
    }

    
    if (tcode == TRANS_OK && !is_valid_cmd(*in_cmd, err_msg)) {
        // We were able to parse the message into a command, it just wasn't structured correctly. 
        // Complain to the player, but do not end the game or toggle players.
        tcode = READ_OK_INVL_CMD;
        char invl_cmd[1000];
        sprintf(invl_cmd, "Invalid command: %s\n", err_msg);
        if (send_command(p->fd, new_invl_cmd(invl_cmd)) == SEND_FAILED) {
            tcode = SEND_FAILED;
        }
    }

    free(err_msg);
    return tcode;
}

