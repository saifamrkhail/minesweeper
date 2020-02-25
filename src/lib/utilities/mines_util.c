/**
* Created by saif on 1/14/20.
*/
///

#include "mines_util.h"
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "../board/board.h"
#include "../board/board_variables.h"
#include "neighbor_util.h"

void place_mines(cell game_brd[ROWS][COLS], int mines[][2]) {
    bool mark[ROWS * COLS];
    memset(mark, false, sizeof(mark));
    // Continue until all random mines have been created.
    for (int i = 0; i < MINES;) {
        int random = rand() % (ROWS * COLS);
        int x = random / ROWS;
        int y = random % COLS;
        if (mark[random] == false) {
            mines[i][0] = x;
            mines[i][1] = y;
            game_brd[mines[i][0]][mines[i][1]].ch = '*';
            mark[random] = true;
            i++;
        }
    }
}

void count_mines(cell game_brd[ROWS][COLS]){
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            game_brd[i][j].ngh_mines = do_counting(i, j, game_brd);
        }
    }
}
