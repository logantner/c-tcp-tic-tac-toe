#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "server_application.h"
#include "pres_layer.h"
#include "command.h"
#include "config.h"

trans_code get_player_response(struct command*, struct player*, struct ttt_game, char*);
void end_game(struct ttt_game, int);
void toggle_player(struct player*, struct ttt_game);

// void set_up_game(int client_fd1, int client_fd2) {

// }


trans_code moderate_game(struct ttt_game game) {
    // Send BEGN to both players
    // Wait to receive response from P1:
    // - MOVE
    //    - Check if move is legal. If not, send INVL and wait for another P1 response
    //    - If move is legal, apply it to the board and check to see if there is a win condition.
    //        - If someone won or tied, send OVER to both and end game
    //        - If the game is not over, send MOVD to both and toggle player. Start over
    // - RSGN
    //    - Send OVER to both players and end the game
    // - DRAW (Start a draw cycle)
    //    - Toggle player. Forward DRAW S to other player
    //    - Get DRAW response from other player.
    //        - If R, toggle player and forward DRAW R. Start over
    //        - If A, send OVER to both players. End game

    // char err_msg[1000];
    char read_fragments[MAX_DATA_PACKET_SIZE];
    trans_code tcode;
    struct command in_cmd;

    // Start game

    if ( send_command(game.p1.fd, new_begn_cmd("X")) || send_command(game.p2.fd, new_begn_cmd("O")) ) {
        // end_game(game, -1);
        return SEND_FAILED;
    }

    struct player cur_p = game.p1;

    while (1) {
        tcode = get_player_response(&in_cmd, &cur_p, game, read_fragments);
        if (tcode != TRANS_OK) {
            return tcode;
        }
        
        // We actually got a fucking command, jesus christ already
        if (in_cmd.code == MOVE) {
            // process move
        } else if (in_cmd.code == RSGN) {
            send_command(cur_p.fd, new_over_cmd("L", "You resigned"));
            toggle_player(&cur_p, game);
            send_command(cur_p.fd, new_over_cmd("W", "Your opponent has resigned"));
            return 0;
        } else if (in_cmd.code == DRAW) {
            if (in_cmd.arg1 == NULL || strcmp(in_cmd.arg1, "S") != 0) {
                send_command(cur_p.fd, new_invl_cmd("Illegal draw request sent"));
            } else {
                // Handle the remainder of the draw bullshit
            }
        } else {
            // The player has sent an illegal command
            char ill_cmd_msg[50];
            sprintf(ill_cmd_msg, "%s is not a legal command to send", code_to_str(in_cmd.code));
        }
        
    }
    
}

void end_game(struct ttt_game game, int err_code) {
    if (err_code < 0) {
        // Do some extra stuff
    }

    close(game.p1.fd);
    close(game.p2.fd);
}

// Points p to whichever player is not him
void toggle_player(struct player* p, struct ttt_game game) {
    if (p->fd == game.p1.fd) {
        *p = game.p2;
    } else {
        *p = game.p1;
    }
}

// Waits for input from the player, parses the message into a command struct, and stores the contents in in_cmd.
// Any read fragments from a previous read should be included in leftovers, which will be rewritten with any new
// read fragment.
trans_code get_player_response(struct command* in_cmd, struct player* p, struct ttt_game game, char* leftovers) {
    char* err_msg = malloc(1000 * sizeof(char));
    trans_code tcode = read_command(p->fd, in_cmd, leftovers, err_msg);

    if (tcode == READ_INVL_MSG) {
        char invl_msg[1000];
        sprintf(invl_msg, "Your message could not even be parsed: '%s'\nEnding game and closing connection", err_msg);
        send_command(p->fd, new_invl_cmd(invl_msg));
        toggle_player(p, game);
        send_command(p->fd, new_over_cmd("D", "Your opponent had a connection issue. Let's call this one a draw."));
    }

    free(err_msg);

    return tcode;
}