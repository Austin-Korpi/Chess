#pragma once
#include "Pieces.h"
#include "Game.h"

int minimax(Game &game, int depth, bool color, int bestChoice);
void call_minimax(Game &game);
void call_minimax_fast(Game& game);