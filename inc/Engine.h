#pragma once
#include "Pieces.h"
#include "Game.h"

int minimax(Game &game, int depth, bool color, int bestChoice);
void take_move(Game &game);
void take_move_fast(Game& game);