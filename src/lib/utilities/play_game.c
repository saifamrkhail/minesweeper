/**
* Created by saif on 1/18/20.
*/
///

#include "play_game.h"
#include "playing_util.h"
#include "mines_util.h"
#include <stdbool.h>
#include <stdio.h>
#include "../boards/board.h"
#include "../boards/board_variables.h"
#include "../validators/validators.h"
#include "../player/player.h"
#include "string_util.h"
#include <stdlib.h>
#include <time.h>
#include <string.h>

void play_game(bool restart) {
    srand(time(NULL));
    player playerx = init_player();
    //expandable array

    //player exits ? yes don't make arry bigger : no make space for new player
    //a funtion to make my array bigger
    int numof_gamers = 1;
    int lenof_name = 30;
    char **gamers = calloc(numof_gamers, sizeof(char *));
    if (gamers) {
        for (int i = 0; i < numof_gamers; ++i) {
            gamers[i] = (char *) calloc(lenof_name, sizeof(char));
        }
        numof_gamers++;
    } else {
        printf("Error, please try starting the game again!");
        exit(EXIT_FAILURE);
    }

    if (!realloc(gamers, sizeof(gamers) * 2)) {
        printf("Error, please try starting the game again!");
        exit(EXIT_FAILURE);
    }

    //search gamer
    size_t str_len = sizeof(gamers) / sizeof(char *);
    qsort((void *) gamers, str_len, sizeof(char *), str_cmp);
    char *target = (char *) bsearch(&playerx.name, gamers, sizeof(gamers), sizeof(char *), str_cmp);
    if (target != NULL) {
        printf("gamer found %s", target);
    } else {
        printf("gamer not found.\n");
    }

    char *file = concat_filename(playerx);
    if ((GAME = fopen(file, "w"))) {
        //fprintf(GAME, "%s",playerx.name);
        bool game_over = false;
        //two boards
        cell hidden_brd[ROWS][COLS], game_brd[ROWS][COLS];
        int mines[MINES][2];
        init_brds(hidden_brd, game_brd);
        place_mines(mines, hidden_brd);
        count_mines(hidden_brd);
        int empty_cells = ROWS * COLS - MINES;
        while (game_over == false) {
            print_brd(game_brd);
            print_rmaining_mines();
            print_hbrd(hidden_brd);
            if (restart) {
                restart_game(game_brd, hidden_brd, mines);
                print_brd(game_brd);
                print_rmaining_mines();
                print_hbrd(hidden_brd);
            }
            move mov = get_move();
            game_over = execute_move(game_brd, hidden_brd, mines, mov);
            if (game_over == false && (OPENED_CELLS == empty_cells || FLAGGED_CORRECT == MINES)) {
                printf("\nYou won !\n");
                playerx.wins++;
                fwrite(&playerx, sizeof(player), 1, GAME);
                fclose(GAME);

                player hafsa;
                file = concat_filename(playerx);
                if ((GAME = fopen(file, "r"))) {
                    if ((fread(&hafsa, sizeof(player), 1, GAME) != 0)) {
                        printf("%s\n", hafsa.name);
                        printf("%d\n", hafsa.wins);
                    } else {
                        fprintf(stderr, "Error, while reading file!\n");
                    }
                    fclose(GAME);
                } else {
                    fprintf(stderr, "Error, while opening file!\n");
                }
                game_over = true;
            }
        }
    } else {
        //call the function again
        fprintf(stderr, "Error, while opening file!\n");
    }
}

void restart_game(cell game_brd[ROWS][COLS], cell hidden_brd[ROWS][COLS], int mines[][2]) {
    //open a random cell
    int random = rand() % (ROWS * COLS) + 1;
    int x = random / ROWS;
    int y = random % COLS;
    move *mov = malloc(sizeof(move));
    if (!mov) {
        printf("ERROR: Out of memory\n");
        exit(EXIT_FAILURE);
    }
    mov->row = x;
    mov->col = y;
    if (game_brd[x][y].ch != '*' && hidden_brd[x][y].ngh_mines != 0) {
        open_cell(game_brd, hidden_brd, mines, *mov);
    } else {
        restart_game(game_brd, hidden_brd, mines);
    }
    free(mov);
}

bool execute_move(cell game_brd[ROWS][COLS], cell hidden_brd[ROWS][COLS], int mines[][2], move mov) {
    /*
    if (game_brd[mov.row][mov.col].s == opened) {
        printf("Already opened, try another cell\n");
        return false;
    }
    if (game_brd[mov.row][mov.col].s == flagged) {
        printf("Already opened, try another cell\n");
        return false;
    }
    */
    if (mov.flag) {
        do_flagging(game_brd, hidden_brd, mines, mov);
    } else {
        bool game_over = open_cell(game_brd, hidden_brd, mines, mov);
        return game_over;
    }
    return false;
}

bool open_cell(cell game_brd[ROWS][COLS], cell hidden_brd[ROWS][COLS],
               int mines[][2], move mov) {
    bool game_over = false;
    if (hidden_brd[mov.row][mov.col].ch == '*' && hidden_brd[mov.row][mov.col].state != flagged) {
        game_brd[mov.row][mov.col].ch = '*';
        for (int i = 0; i < MINES; i++) {
            game_brd[mines[i][0]][mines[i][1]].ch = '*';
        }
        print_brd(game_brd);
        printf("\nYou lost!\n");
        game_over = true;
    } else {
        int count = hidden_brd[mov.row][mov.col].ngh_mines;
        game_brd[mov.row][mov.col].ngh_mines = count;
        game_brd[mov.row][mov.col].state = opened;
        OPENED_CELLS++;
        if (!count) {
            open_ngh(game_brd, hidden_brd, mov);
        }
    }
    return game_over;
}

void do_flagging(cell game_brd[ROWS][COLS], cell hidden_brd[ROWS][COLS],
                 int mines[][2], move mov) {
    hidden_brd[mov.row][mov.col].state = flagged;
    if (hidden_brd[mov.row][mov.col].ch == '*') {
        FLAGGED_CORRECT++;
        game_brd[mov.row][mov.col].ch = '*';
    } else {
        int count = hidden_brd[mov.row][mov.col].ngh_mines;
        game_brd[mov.row][mov.col].ngh_mines = count;
        game_brd[mov.row][mov.col].state = flagged;
        FLAGGED_WRONG++;
        if (!count) {
            open_ngh(game_brd, hidden_brd, mov);
        }
    }
}

void open_ngh(cell game_brd[ROWS][COLS], cell hidden_brd[ROWS][COLS], move mov) {
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
                    int count = hidden_brd[temp->row][temp->col].ngh_mines;
                    game_brd[temp->row][temp->col].ngh_mines = count;
                    game_brd[temp->row][temp->col].state = opened;
                    OPENED_CELLS++;
                    if (!count) {
                        open_ngh(game_brd, hidden_brd, *temp);
                    }
                }
            }
        }
    }
    free(temp);
}


