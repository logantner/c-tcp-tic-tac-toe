#include <stdio.h>
#include "ttt_game.h"

void resetBoard(char[3][3]);
void displayBoard(char[3][3]);

////////////////////////////////

int game() {
    char board[3][3];
    resetBoard(board);
    displayBoard(board);
    printf("==================\n");
    board[0][2] = 'O';
    board[1][1] = 'X';
    displayBoard(board);

    return 0;
}

void resetBoard(char board[3][3]) {
    for (int i=0; i<3; ++i) {
        for (int j=0; j<3; ++j) {
            board[i][j] = ' ';
        }
    }
}

void displayBoard(char board[3][3]) {
    for (int i=0; i<3; ++i) {
        if (i == 0) {
            printf("+-----------+\n");
        } else {
            printf("|---+---+---|\n");
        }
        
        for (int j=0; j<3; ++j) {
            printf("| %c ", board[i][j]);
        }
        printf("|\n");
    }
    printf("+-----------+\n");
}