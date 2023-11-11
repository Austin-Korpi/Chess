#pragma once
#include "Pieces.h"
#include "Game.h"

move_info move_with_opening(Game& game, move_info (*func)(Game&));
move_info call_minimax(Game &game);
move_info call_minimax_fast(Game& game);
move_info call_minimax_IDS(Game &game);
move_info call_minimax_IDS_fast(Game &game);