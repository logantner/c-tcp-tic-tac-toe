#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "game.h"
#include "config.h"

char check_win_condition(struct ttt_game, int, int, int);
int is_board_full(struct ttt_game);


struct ttt_game new_game() {
    struct ttt_game game;

    strcpy(game.board, ".........");
    game.is_active = 0;
    game.p1 = new_player();
    game.p2 = new_player();

    game.p1.role = 'X';
    game.p2.role = 'O';

    return game;
}

struct player new_player() {
    struct player p;
    memset(&p, 0, sizeof(struct player));
    p.fd = 0;
    p.msg_fragment = malloc((MAX_DATA_PACKET_SIZE+1) * sizeof(char));

    return p;
}


void free_player(struct player p) {
    if (p.name != NULL) {
        free(p.name);
        p.name = NULL;
    }

    if (p.msg_fragment != NULL) {
        free(p.msg_fragment);
        p.msg_fragment = NULL;
    }
}

// Assigns fd, message fragments, and player name to a game player (randomly if two options are available)
void add_player(struct ttt_game* game, struct player newp, char* pname) {
    struct player* p;
    
    // If one player is assigned, choose the other
    if (game->p1.fd != 0) {
        p = &(game->p2);
    } else if (game->p2.fd != 0) {
        p = &(game->p1);
    } 
    // Otherwise, randomly assign who will be going first
    else {
        p = (rand() % 2 == 0) ? &(game->p1) : &(game->p2);
    }
    
    p->fd = newp.fd;
    strcpy(p->msg_fragment, newp.msg_fragment);
    p->name = strdup(pname);
}

int num_players(struct ttt_game game) {
    return (game.p1.fd != 0) + (game.p2.fd != 0);
}

char get_board_val(struct ttt_game game, int r, int c) {
    int flat_coord = 3 * (r-1) + (c-1);
    return game.board[flat_coord];
}

void set_board_val(struct ttt_game* game, int r, int c, char role) {
    int flat_coord = 3 * (r-1) + (c-1);
    game->board[flat_coord] = role;
}

// Returns a string representation of the winner if one has been declared, "T" if the
// board has been filled with no winner, or null otherwise.
char* get_win_status(struct ttt_game game) {
    char win_coords[8][3] = {
        {0,1,2}, {3,4,5}, {6,7,8},  // horizontals
        {0,3,6}, {1,4,7}, {2,5,8},  // verticals
        {0,4,8}, {2,4,6}            // diagonals
    };

    // check for winner
    int c1, c2, c3;
    char winner;
    for (int i=0; i<8; ++i) {
        c1 = win_coords[i][0];
        c2 = win_coords[i][1];
        c3 = win_coords[i][2];
        winner = check_win_condition(game, c1, c2, c3);
        if (winner != '.') {
            return winner == 'X' ? "X" : "O";
        }
    }

    // check for tie game
    if (is_board_full(game)) {
        return "T";
    }

    // the game is not ready to end
    return NULL;
}

// Returns the winning player role if a win was detected at provided flat coords, '.' otherwise
char check_win_condition(struct ttt_game game, int c1, int c2, int c3) {
    if (game.board[c1] == game.board[c2] && game.board[c2] == game.board[c3]) {
        return game.board[c1];
    }
    return '.';
}

int is_board_full(struct ttt_game game) {
    for (int i = 0; i < 9; ++i) {
        if (game.board[i] == '.') {
            return 0;
        }
    }
    return 1;
}

void display_board(char board[10]) {
    for (int i=0; i<3; ++i) {
        if (i == 0) {
            printf("+-----------+\n");
        } else {
            printf("|---+---+---|\n");
        }
        
        for (int j=0; j<3; ++j) {
            printf("| %c ", board[3*i + j]);
        }
        printf("|\n");
    }
    printf("+-----------+\n");
}

void display_player(struct player p) {
    printf("===================\n");
    printf("  Player info:\n");
    printf("  fd: %d\n", p.fd);
    printf("  name: %s\n", p.name);
    printf("  role: %c\n", p.role);
    printf("  message frags: %s\n", p.msg_fragment);
    printf("===================\n");
}