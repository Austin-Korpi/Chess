#include "Pieces.h"
#include "Game.h"

using namespace std;


bool check_ob(position move) {
	if ((move.x < 0) || (move.x > 7) ||
		(move.y < 0) || (move.y > 7)) {
		return true;
	}
	return false;
}

bool has_moved(Piece* piece, std::vector<move_info> &move_log) {
	for (const auto& log : move_log) {
		if (log.piece == piece) {
			return true;
		}
	}
	return false;
}

Piece::Piece(int i, pieceOptions pc,bool wht, int hor, int ver) {
	id = i;
	piece = pc;
	white = wht;
	x = hor;
	y = ver;
}

Pawn::Pawn    (int i, bool wht, int hor, int ver) : Piece(i, pawn,   wht, hor, ver) {}
Night::Night  (int i, bool wht, int hor, int ver) : Piece(i, night,  wht, hor, ver) {}
Bishop::Bishop(int i, bool wht, int hor, int ver) : Piece(i, bishop, wht, hor, ver) {}
Rook::Rook    (int i, bool wht, int hor, int ver) : Piece(i, rook,   wht, hor, ver) {}
Queen::Queen  (int i, bool wht, int hor, int ver) : Piece(i, queen,  wht, hor, ver) {}
King::King    (int i, bool wht, int hor, int ver) : Piece(i, king,   wht, hor, ver) {}

bool Piece::can_capture(Piece* other) {
	if (other == NULL) {
		return false;
	}
	if (other->white == white) {
		return false;
	}
	if (other->piece == king) {
		return false;
	}
	return true;
}

int Pawn::find_valid_moves(Game &game, position (&moves)[27]) {
	int numMoves = 0;
	int direction = white * (-2) + 1;
	//Basic Move
	if (!check_ob({ x,y + direction }) && game.board[y + direction][x] == NULL){
		if (!game.leap_then_look(this, { x,y + direction }, white ? game.whiteKing : game.blackKing)) {

			moves[numMoves] = { x, y + direction };
			numMoves++;
			//Move two
		}
		if (!check_ob({ x,y + 2 * direction }) && game.board[y + 2 * direction][x] == NULL && !has_moved(this, game.move_log)
			&& !game.leap_then_look(this, { x,y + 2 * direction }, white ? game.whiteKing : game.blackKing)) {
			moves[numMoves] = { x, y + 2 * direction };
			numMoves++;
		}
		
	}

	//Diagonal
	for (position move : {position{ x + 1 ,y + direction }, position{ x - 1 ,y + direction } }) {
		if (!check_ob(move)) {
			if (can_capture(game.board[move.y][move.x]) 
				&& !game.leap_then_look(this, move, white ? game.whiteKing : game.blackKing)) {

				moves[numMoves] = move;
				numMoves++;
			}
			//En passant
			else if (can_capture(game.board[y][move.x]) && game.board[y][move.x]->piece == pawn) {
				move_info lastMove = game.move_log[game.move_log.size() - 1];
				if (lastMove.piece->piece == pawn && abs(lastMove.from.y - lastMove.to.y) > 1 
					&& !game.leap_then_look(this, move, white ? game.whiteKing : game.blackKing)) {

					moves[numMoves] = move;
					numMoves++;
				}
			}
		}
	}

	return numMoves;
}

int Night::find_valid_moves(Game &game, position (&moves)[27]) {
	int numMoves = 0;
	position possibilities[] = { {x + 2, y - 1}, {x + 2, y + 1},
		{x + 1, y + 2}, {x - 1, y + 2}, {x - 2, y + 1},
		{x - 2, y - 1}, {x - 1, y - 2}, {x + 1, y - 2} };
	for (int i = 0; i < 8; i++) {
		position move = possibilities[i];
		if (!check_ob(move) && (game.board[move.y][move.x] == NULL || can_capture(game.board[move.y][move.x])) 
			&& !game.leap_then_look(this, move, white ? game.whiteKing : game.blackKing)) {

			moves[numMoves] = move;
			numMoves++;
		}
	}
	return numMoves;
}

int Bishop::find_valid_moves(Game &game, position (&moves)[27]) {
	int numMoves = 0;
	position dir[4] = { {1,1},{-1,1},{-1,-1},{1,-1} };
	for (int i = 0; i < 4; i++) {
		int potX = x + dir[i].x;
		int potY = y + dir[i].y;
		while (potX > -1 && potY > -1 && potX < 8 && potY < 8) {
			if (game.board[potY][potX] == NULL ){
				if (!game.leap_then_look(this, { potX, potY }, white ? game.whiteKing : game.blackKing)) {
					moves[numMoves] = { potX, potY };
					numMoves++;
				}
				potX += dir[i].x;
				potY += dir[i].y;
			}
			else {
				if (can_capture(game.board[potY][potX])
					&& !game.leap_then_look(this, { potX, potY }, white ? game.whiteKing : game.blackKing)) {

					moves[numMoves] = { potX, potY };
					numMoves++;
				}
				break;
			}
		}

	}
	return numMoves;
}

int Rook::find_valid_moves(Game &game, position (&moves)[27]) {
	int numMoves = 0;
	position dir[4] = { {1,0},{-1,0},{0,1},{0,-1} };
	for (int i = 0; i < 4; i++) {
		int potX = x + dir[i].x;
		int potY = y + dir[i].y;
		while (potX > -1 && potY > -1 && potX < 8 && potY < 8) {
			if (game.board[potY][potX] == NULL){
				if (!game.leap_then_look(this, { potX, potY }, white ? game.whiteKing : game.blackKing)) {

					moves[numMoves] = { potX, potY };
					numMoves++;
				}
				potX += dir[i].x;
				potY += dir[i].y;
			}
			else {
				if (can_capture(game.board[potY][potX]) && !game.leap_then_look(this, { potX, potY }, white ? game.whiteKing : game.blackKing)) {

					moves[numMoves] = { potX, potY };
					numMoves++;
				}
				break;
			}
		}

	}
	return numMoves;
}

int Queen::find_valid_moves(Game &game, position (&moves)[27]) {
	int numMoves = 0;
	position dir[8] = { {1,1},{-1,1},{-1,-1},{1,-1},
		{1,0},{-1,0},{0,1},{0,-1} };
	for (int i = 0; i < 8; i++) {
		int potX = x + dir[i].x;
		int potY = y + dir[i].y;
		while (potX > -1 && potY > -1 && potX < 8 && potY < 8) {
			if (game.board[potY][potX] == NULL){
				if (!game.leap_then_look(this, { potX, potY }, white ? game.whiteKing : game.blackKing)) {

					moves[numMoves] = { potX, potY };
					numMoves++;
				}
				potX += dir[i].x;
				potY += dir[i].y;
			}
			else {
				if (can_capture(game.board[potY][potX]) && !game.leap_then_look(this, { potX, potY }, white ? game.whiteKing : game.blackKing)) {

					moves[numMoves] = { potX, potY };
					numMoves++;
				}
				break;
			}
		}
	}
	return numMoves;
}

int King::find_valid_moves(Game &game, position (&moves)[27]) {
	int numMoves = 0;
	position possibilities[8] = { {x + 1,y + 1},{x + 1,y},{x + 1, y - 1},
		{x, y + 1}, {x,y - 1}, {x - 1, y + 1},{x - 1, y},{x - 1,y - 1} };
	for(position move:possibilities){
		if (!check_ob(move) && (game.board[move.y][move.x] == NULL || can_capture(game.board[move.y][move.x]))
			&& !game.leap_then_look(this, move, this)) {

			moves[numMoves] = move;
			numMoves++;
		}
	}

	//Castling
	if (!has_moved(this, game.move_log) && !game.check_for_check(white, { x, y })){
		if (game.board[y][7] != NULL && game.board[y][7]->piece == rook && !has_moved((game.board[y][7]), game.move_log)
			&& game.board[y][5] == NULL && game.board[y][6] == NULL && !game.check_for_check(white, { 5, y })) {
			moves[numMoves] = {  6, y };
			numMoves++;
		}
		if (game.board[y][0] != NULL && game.board[y][0]->piece == rook && !has_moved(game.board[y][0], game.move_log)
			&& game.board[y][1] == NULL && game.board[y][2] == NULL && game.board[y][3] == NULL && !game.check_for_check(white, { 3, y })) {
			moves[numMoves] = { 2, y };
			numMoves++;
		}

	}
	
	return numMoves; 
}
