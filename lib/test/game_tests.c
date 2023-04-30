#include <stdio.h>
#include <string.h>

#include "game_tests.h"
#include "../game.h"

void test_get_win_status(char*);

void run_all_game_tests() {
    // test_new_game();
    // test_board_spaces();
    // run_get_win_status_tests();
    test_adding_players();
}

void test_new_game() {
    printf("=========== Running new_game test ===========\n");
    struct ttt_game game = new_game();
    printf("Active status: %d\n", game.is_active);
    display_player(game.p1);
    display_player(game.p2);
    display_board(game.board);
}

void test_board_spaces() {
    printf("=========== Running board space tests ===========\n");
    struct ttt_game game = new_game();
    set_board_val(&game, 1, 1, 'X');
    set_board_val(&game, 2, 2, 'O');
    set_board_val(&game, 3, 2, 'X');
    printf("Setting board(1,1) to X\n");
    printf("Setting board(2,2) to O\n");
    printf("Setting board(3,2) to X\n");
    display_board(game.board);
    printf("board(1,1) value: %c\n", get_board_val(game, 1, 1));
    printf("board(1,3) value: %c\n", get_board_val(game, 1, 3));
    printf("board(2,2) value: %c\n", get_board_val(game, 2, 2));
    printf("board(3,2) value: %c\n", get_board_val(game, 3, 2));
    printf("board(3,3) value: %c\n", get_board_val(game, 3, 3));
}

void run_get_win_status_tests() {
    printf("========== GET_WIN_STATUS TESTS ==========\n");
    char tests[][10] = {
        ".........",    // Empty unfinished
        "XOOO.OXOX",    // Nontrivial unfinished
        "XXOOOXXOX",    // Draw
        "XO.OXO.OX",    // Diagonal win
        "X...X.OOO",    // Horizontal win
        ".XO.X..XO",    // Vertical win
        "XOXOXOOOX"     // Full board win
    };

    int ntests = sizeof(tests)/sizeof(tests[0]);
    for (int i=0; i<ntests; ++i) {
        test_get_win_status(tests[i]);
    }

}

void test_get_win_status(char* board) {
    struct ttt_game game = new_game();
    strcpy(game.board, board);
    display_board(game.board);
    char* win_status = get_win_status(game) == NULL ? "none" : get_win_status(game);
    printf("Game winner: %s\n", win_status);
}

void test_adding_players() {
    struct ttt_game game = new_game();
    printf("New game has %d players to start\n", num_players(game));

    struct player p_a = new_player();
    p_a.fd = 11;
    char* name_a = strdup("Name A");
    add_player(&game, p_a, name_a);

    printf("** Added player A **\n");
    printf("Game now has %d players:\n", num_players(game));
    display_player(game.p1);
    display_player(game.p2);

    struct player p_b = new_player();
    p_b.fd = 12;
    char* name_b = strdup("Name B");
    add_player(&game, p_b, name_b);

    printf("** Added player B **\n");
    printf("Game now has %d players:\n", num_players(game));
    display_player(game.p1);
    display_player(game.p2);

    printf("** Freeing players **\n");
    free_player(game.p1);
    free_player(game.p2);
}

