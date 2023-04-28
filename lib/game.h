#ifndef GAME_H_
#define GAME_H_

struct player {
    int fd;
    char* name;
    char role;          // X or O
    char* msg_fragment; // Any message spillover to be processed later
};

struct ttt_game {
    struct player p1; // p1 is always X
    struct player p2; // p2 is always O
    char board[10];
    int is_active;
};

struct ttt_game new_game(struct player, struct player);
char get_board_val(struct ttt_game, int, int);
void set_board_val(struct ttt_game, int, int, char);
char* get_win_status(struct ttt_game);

#endif