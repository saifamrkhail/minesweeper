/**
* Created by saif on 1/18/20.
*/
///

#include "game_util.h"
#include "playing_util.h"
#include "mines_util.h"
#include <stdbool.h>
#include <stdio.h>
#include "../board/board.h"
#include "../board/board_variables.h"
#include "../validators/validators.h"
#include "../player/player.h"
#include "../service/set/setservice.h"
#include "../service/get/getservice.h"
#include "string_util.h"
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <limits.h>

void play_game(char *answer, char *player_file_path) {
    bool aborted_game_loaded = false;
    cell game_brd[ROWS][COLS];
    if (answer) {
        if (strcmp(answer, "y") == 0 || strcmp(answer, "yes") == 0) {
            aborted_game_loaded = set_board(game_brd, player_file_path);
            if (!aborted_game_loaded) {
                printf("aborted game could not be loaded\n"
                       "but a new game was created for you\n");

            }
        }
    }
    GAME = fopen(player_file_path, "w");
    if (GAME) {
        int mines[MINES][2];
        /*if a game was not loaded then game_brd is empty. it need to be filled
         * in case of restart loaded has to be false. because the board need to be prepared fo a new game*/
        if (!aborted_game_loaded) {
            init_brd(game_brd);
            place_mines(game_brd, mines);
            count_mines(game_brd);
        }
        int empty_cells = ROWS * COLS - MINES;
        bool game_over = false;
        move mov = {false, -1, -1, false, false};
        while (game_over == false) {
            print_brd(game_brd);
            print_rmaining_mines();
            print_mbrd(game_brd);
            mov = get_move();
            if (mov.restart) {
                restart_game(game_brd, mines);
                continue;
            }
            if (mov.abort) {
                save_game(game_brd, mov.abort);
            }
            game_over = execute_move(game_brd, mines, mov);
            //in case of winning the game
            if (game_over == false && (OPENED_CELLS == empty_cells || FLAGGED_CORRECT == MINES)) {
                print_brd(game_brd);
                print_rmaining_mines();
                printf("You won!\n");
                PLAYERX.wins++;
                PLAYERX.games++;
                save_game(game_brd, mov.abort);
            }
        }
        //in case of losing the game
        save_game(game_brd, mov.abort);
    } else {
        printf("Error, while opening %s's file!\n"
               "exiting game\n", PLAYERX.name);
        exit(EXIT_FAILURE);
    }
}

void save_game(cell game_brd[ROWS][COLS], bool abort) {
    FLAGGED_TOTAL = FLAGGED_CORRECT + FLAGGED_WRONG;
    PLAYERX.cells += (OPENED_CELLS + FLAGGED_TOTAL);
    PLAYERX.info.aborted = abort;
    if (abort) {
        PLAYERX.info.rows = ROWS;
        PLAYERX.info.cols = COLS;
        PLAYERX.info.mines = MINES;
    }
    if (fwrite(&PLAYERX, sizeof(player), 1, GAME) != 1) {
        printf("Error, while saving the player!");
        exit(EXIT_FAILURE);
    }
    if (abort) {
        for (int i = 0; i < ROWS; ++i) {
            for (int j = 0; j < COLS; ++j) {
                if (fwrite(&game_brd[i][j], sizeof(cell), 1, GAME) == 0) {
                    printf("Error, while saving the game!");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
    fclose(GAME);
    exit(EXIT_SUCCESS);
}

bool execute_move(cell game_brd[ROWS][COLS], int mines[][2], move mov) {
    bool game_over = false;
    if (game_brd[mov.row][mov.col].state == opened || game_brd[mov.row][mov.col].state == flagged) {
        printf("Already opened, try another cell\n");
    } else {
        if (mov.flag) {
            flag_cell(game_brd, mov);
        } else {
            game_over = open_cell(game_brd, mines, mov);
        }
    }
    return game_over;
}

bool open_cell(cell game_brd[ROWS][COLS], int mines[][2], move mov) {
    bool game_over = false;
    if (game_brd[mov.row][mov.col].ch == '*' && game_brd[mov.row][mov.col].state != flagged) {
        PLAYERX.losses++;
        PLAYERX.games++;
        game_brd[mov.row][mov.col].ch = '*';
        for (int i = 0; i < MINES; i++) {
            game_brd[mines[i][0]][mines[i][1]].ch = '*';
        }
        print_brd(game_brd);
        printf("You lost!\n");
        game_over = true;
    } else {
        int count = game_brd[mov.row][mov.col].ngh_mines;
        game_brd[mov.row][mov.col].ngh_mines = count;
        game_brd[mov.row][mov.col].state = opened;
        OPENED_CELLS++;
        if (!count) {
            open_ngh(game_brd, mov);
        }
    }
    return game_over;
}

void flag_cell(cell game_brd[ROWS][COLS], move mov) {
    game_brd[mov.row][mov.col].state = flagged;
    if (game_brd[mov.row][mov.col].ch == '*') {
        FLAGGED_CORRECT++;
        game_brd[mov.row][mov.col].ch = '*';
    } else {
        int count = game_brd[mov.row][mov.col].ngh_mines;
        game_brd[mov.row][mov.col].state = flagged;
        FLAGGED_WRONG++;
        if (!count) {
            open_ngh(game_brd, mov);
        }
    }
}

void open_ngh(cell game_brd[ROWS][COLS], move mov) {
    int neighbors[8][2] = {{-1, -1},
                           {-1, 0},
                           {-1, 1},
                           {0,  -1},
                           {0,  1},
                           {1,  1},
                           {1,  -1},
                           {1,  0}};
    move *temp = malloc(sizeof(move));
    if (!temp) {
        printf("ERROR: Out of memory\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 1; ++j) {
            temp->row = mov.row + neighbors[i][j];
            temp->col = mov.col + neighbors[i][j + 1];
            if (is_cell_valid(temp->row, temp->col)) {
                if (game_brd[temp->row][temp->col].state == hidden) {
                    int count = game_brd[temp->row][temp->col].ngh_mines;
                    game_brd[temp->row][temp->col].state = opened;
                    OPENED_CELLS++;
                    if (!count) {
                        open_ngh(game_brd, *temp);
                    }
                }
            }
        }
    }
    free(temp);
}

void restart_game(cell game_brd[ROWS][COLS], int mines[][2]) {
    printf("\nnew game\n");
    init_brd(game_brd);
    place_mines(game_brd, mines);
    count_mines(game_brd);
    open_randomcell(game_brd, mines);
}

void open_randomcell(cell game_brd[ROWS][COLS], int mines[][2]) {
    srand(time(NULL));
    int random = rand() % (ROWS * COLS) + 1;
    int x = random / ROWS;
    int y = random % COLS;
    move *mov = malloc(sizeof(move));
    if (!mov) {
        printf("Error, out of memory\n");
        exit(EXIT_FAILURE);
    }
    mov->row = x;
    mov->col = y;
    if (game_brd[x][y].ch != '*' && game_brd[x][y].ngh_mines != 0) {
        open_cell(game_brd, mines, *mov);
        PLAYERX.cells++;
    } else {
        open_randomcell(game_brd, mines);
    }
    free(mov);
}

void help() {
    printf("opening help instructions\n");
    char minespr_path[PATH_MAX + 1] = {'\0'};
    char *target = "minespr";
    char *result = realpath(target, minespr_path);
    if (!result) {
        strcpy(minespr_path, get_path(target));
    } else {
        strcat(minespr_path, "/");
    }
    char *file_name = "help.txt";
    bool existent = is_existent(minespr_path, file_name);
    if (existent) {
        char *file_path = concat_filepath(minespr_path, file_name);
        int len_file_path = strlen(file_path);

        char *chmod = "chmod 444 ";
        char chmod_buf[len_file_path + strlen(chmod)];
        strcpy(chmod_buf, chmod);
        strcat(chmod_buf, file_path);
        system(chmod_buf);

        char *vim = "vim ";
        char vim_buf[len_file_path + strlen(vim)];
        strcpy(vim_buf, vim);
        strcat(vim_buf, file_path);
        system(vim_buf);

        system("clear");
    } else {
        printf("Error, help instructions not found!");
    }

}
