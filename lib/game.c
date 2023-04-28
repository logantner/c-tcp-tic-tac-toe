#include <string.h>
#include <stdlib.h>

#include "game.h"

char check_win_condition(struct ttt_game, int, int, int);
int is_board_full(struct ttt_game);


struct ttt_game new_game(struct player p1, struct player p2) {
    struct ttt_game game;

    strcpy(game.board, ".........");

    // Randomly assign who will be going first
    if (rand() % 2 == 0) {
        game.p1 = p1;
        game.p2 = p2;
    } else {
        game.p2 = p1;
        game.p1 = p2;
    }

    game.is_active = 1;

    return game;
}

char get_board_val(struct ttt_game game, int r, int c) {
    int flat_coord = 3 * (r-1) + (c-1);
    return game.board[flat_coord];
}

void set_board_val(struct ttt_game game, int r, int c, char role) {
    int flat_coord = 3 * (r-1) + (c-1);
    game.board[flat_coord] = role;
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