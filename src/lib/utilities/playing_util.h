/**
* Created by saif on 1/14/20.
*/
///

#ifndef MINESPR_PLAYING_UTIL_H
#define MINESPR_PLAYING_UTIL_H

#include <stdbool.h>
#include "../playing_board/board.h"
int *retrieve_move(char *str, int count);
int *make_move(int row, int col);

bool play_recursiv(
        char **game_board,
        char **hidden_board,
        int **mines,
        int row, int col,
        int *remaining_moves);
#endif //MINESPR_PLAYING_UTIL_H
