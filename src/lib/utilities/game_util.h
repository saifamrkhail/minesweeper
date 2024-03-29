/**\file game_util.h
 * \file functions to play the game
* Created by saif on 1/18/20.
*/
///

#ifndef MINESPR_GAME_UTIL_H
#define MINESPR_GAME_UTIL_H

#include <stdbool.h>
#include "../board/board.h"

typedef struct {
    bool win;
    bool loss;
    bool abort;
    bool error;
} game_result;

typedef struct {
    bool game_over;
    bool error;
} check;

void restart_game(cell game_brd[ROWS][COLS]);

game_result play_game(char *answer, char *player_file_path);

bool save_game(cell game_brd[ROWS][COLS], bool abort);

check open_cell(cell game_brd[ROWS][COLS], move mov);

bool open_ngh(cell game_brd[ROWS][COLS], move mov);

bool flag_cell(cell game_brd[ROWS][COLS], move mov);

check execute_move(cell game_brd[ROWS][COLS], move mov);

void open_randomcell(cell game_brd[ROWS][COLS]);

void help();

#endif //MINESPR_GAME_UTIL_H
