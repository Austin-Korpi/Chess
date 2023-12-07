#pragma once
#include "Pieces.h"
#include "Game.h"

#define MAXDEPTH 7

Move move_with_opening(Game& game, Move (*func)(Game&));
Move call_minimax(Game &game);
Move call_minimax_fast(Game& game);
Move call_minimax_IDS(Game &game);
Move call_minimax_IDS_fast(Game &game);
int minimax(Game &, int, int, int, Move*, bool);
void printStats();
void clearTable();
int evaluate_board(Game& game);