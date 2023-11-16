#pragma once
#include "Pieces.h"
#include "Game.h"

#define MAXDEPTH 5

move_info move_with_opening(Game& game, move_info (*func)(Game&));
move_info call_minimax(Game &game);
move_info call_minimax_fast(Game& game);
move_info call_minimax_IDS(Game &game);
move_info call_minimax_IDS_fast(Game &game);
int minimax(Game &game, int depth, int alpha, int beta, move_info* choice);
void printStats();
void clearTable();