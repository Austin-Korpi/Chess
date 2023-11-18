#include "Pieces.h"
#include "Game.h"

using namespace std;


bool check_ob(Position move) {
	if ((move.x < 0) || (move.x > 7) ||
		(move.y < 0) || (move.y > 7)) {
		return true;
	}
	return false;
}

Piece::Piece() {
	x = y = 0;
	white = true;
	type = pawn;
	captured = false;
}

Piece::Piece(pieceOptions tp,bool wht, unsigned char hor, unsigned char ver) {
	type = tp;
	white = wht;
	x = hor;
	y = ver;
	captured = false;
}

bool Piece::can_capture(Piece* other) {
	if (other == NULL) {
		return false;
	}
	if (other->white == white) {
		return false;
	}
	if (other->type == king) {
		return false;
	}
	return true;
}

int Piece::find_valid_moves(Game &game, Position (&moves)[27]) {
	int numMoves = 0;

	if (type == pawn) {
		int direction = white * (-2) + 1;
		//Basic Move
		if (!check_ob({ x,y + direction }) && game.board[y + direction][x] == NULL){
			if (!game.leap_then_look(this, { x,y + direction })) {

				moves[numMoves] = { x, y + direction };
				numMoves++;
				//Move two
			}
			if (!check_ob({ x,y + 2 * direction }) && game.board[y + 2 * direction][x] == NULL && ((white && y==6) || (!white && y==1))
				&& !game.leap_then_look(this, { x,y + 2 * direction })) {
				moves[numMoves] = { x, y + 2 * direction };
				numMoves++;
			}
			
		}

		//Diagonal
		for (Position move : {Position{ x + 1 ,y + direction }, Position{ x - 1 ,y + direction } }) {
			if (!check_ob(move)) {
				if (can_capture(game.board[move.y][move.x]) 
					&& !game.leap_then_look(this, move)) {

					moves[numMoves] = move;
					numMoves++;
				}
				//En passant
				else if (can_capture(game.board[y][move.x]) && game.board[y][move.x]->type == pawn) {
					Move lastMove = game.moveLog[game.moveLog.size() - 1];
					if (lastMove.to == Position{move.x, y} && game.board[lastMove.to.y][lastMove.to.x]->type == pawn && abs(lastMove.from.y - lastMove.to.y) > 1 
						&& !game.leap_then_look(this, move)) {

						moves[numMoves] = move;
						numMoves++;
					}
				}
			}
		}
	} else if (type == night) {
		Position possibilities[] = { {x + 2, y - 1}, {x + 2, y + 1},
			{x + 1, y + 2}, {x - 1, y + 2}, {x - 2, y + 1},
			{x - 2, y - 1}, {x - 1, y - 2}, {x + 1, y - 2} };
		for (int i = 0; i < 8; i++) {
			Position move = possibilities[i];
			if (!check_ob(move) && (game.board[move.y][move.x] == NULL || can_capture(game.board[move.y][move.x])) 
				&& !game.leap_then_look(this, move)) {

				moves[numMoves] = move;
				numMoves++;
			}
		}
	} else if (type == bishop) {
		Position dir[4] = { {1,1},{-1,1},{-1,-1},{1,-1} };
		for (int i = 0; i < 4; i++) {
			int potX = x + dir[i].x;
			int potY = y + dir[i].y;
			while (potX > -1 && potY > -1 && potX < 8 && potY < 8) {
				if (game.board[potY][potX] == NULL ){
					if (!game.leap_then_look(this, { potX, potY })) {
						moves[numMoves] = { potX, potY };
						numMoves++;
					}
					potX += dir[i].x;
					potY += dir[i].y;
				}
				else {
					if (can_capture(game.board[potY][potX])
						&& !game.leap_then_look(this, { potX, potY })) {

						moves[numMoves] = { potX, potY };
						numMoves++;
					}
					break;
				}
			}
		}
	} else if (type == rook) {
		Position dir[4] = { {1,0},{-1,0},{0,1},{0,-1} };
		for (int i = 0; i < 4; i++) {
			int potX = x + dir[i].x;
			int potY = y + dir[i].y;
			while (potX > -1 && potY > -1 && potX < 8 && potY < 8) {
				if (game.board[potY][potX] == NULL){
					if (!game.leap_then_look(this, { potX, potY })) {

						moves[numMoves] = { potX, potY };
						numMoves++;
					}
					potX += dir[i].x;
					potY += dir[i].y;
				}
				else {
					if (can_capture(game.board[potY][potX]) && !game.leap_then_look(this, { potX, potY })) {

						moves[numMoves] = { potX, potY };
						numMoves++;
					}
					break;
				}
			}

		}
	} else if (type == queen) {
		Position dir[8] = { {1,1},{-1,1},{-1,-1},{1,-1},
			{1,0},{-1,0},{0,1},{0,-1} };
		for (int i = 0; i < 8; i++) {
			int potX = x + dir[i].x;
			int potY = y + dir[i].y;
			while (potX > -1 && potY > -1 && potX < 8 && potY < 8) {
				if (game.board[potY][potX] == NULL){
					if (!game.leap_then_look(this, { potX, potY })) {

						moves[numMoves] = { potX, potY };
						numMoves++;
					}
					potX += dir[i].x;
					potY += dir[i].y;
				}
				else {
					if (can_capture(game.board[potY][potX]) && !game.leap_then_look(this, { potX, potY })) {

						moves[numMoves] = { potX, potY };
						numMoves++;
					}
					break;
				}
			}
		}
	} else {
		Position possibilities[8] = { {x + 1,y + 1},{x + 1,y},{x + 1, y - 1},
			{x, y + 1}, {x,y - 1}, {x - 1, y + 1},{x - 1, y},{x - 1,y - 1} };
		for(Position move:possibilities){
			if (!check_ob(move) && (game.board[move.y][move.x] == NULL || can_capture(game.board[move.y][move.x]))
				&& !game.leap_then_look(this, move)) {

				moves[numMoves] = move;
				numMoves++;
			}
		}

		//Castling
		if (!game.check_for_check(white, { x, y })) {
			// King side
			if (game.board[y][7] != NULL && game.board[y][7]->type == rook && 
				// ((white && game.castleK) || (!white && game.castlek)) &&
				((white && game.canCastle & 0b1000) || (!white && game.canCastle & 0b0010)) &&
				game.board[y][5] == NULL && game.board[y][6] == NULL && 
				!game.leap_then_look(this, {5,y}) && !game.leap_then_look(this, {6, y})) {
				moves[numMoves] = {  6, y };
				numMoves++;
			}
			// Queen side
			if (game.board[y][0] != NULL && game.board[y][0]->type == rook && 
				// ((white && game.castleQ) || (!white && game.castleq)) &&
				((white && game.canCastle & 0b0100) || (!white && game.canCastle & 0b0001)) &&
				game.board[y][1] == NULL && game.board[y][2] == NULL && game.board[y][3] == NULL 
				&& !game.leap_then_look(this, {3,y}) && !game.leap_then_look(this, {2, y})) {
				moves[numMoves] = { 2, y };
				numMoves++;
			}
		}
	}
	return numMoves;
}
