#ifndef ENGINE_H
#define ENGINE_H

#include "Pieces.h"
#include "Game.h"

#define MAX_DEPTH 7
#define MAX_RUN_TIME 3

void start_timer(int seconds);

Move move_with_opening(Game &game, Move (*func)(Game &));
Move call_minimax(Game &game);
Move call_minimax_fast(Game &game);
Move call_minimax_IDS(Game &game);
Move call_minimax_IDS_fast(Game &game);
int minimax(Game &, int, int, int, Move *, bool);
void clear_table();
int evaluate_board(Game &game);

#endif