#pragma once
#include "Pieces.h"
#include <vector>
#include <string>




class Game {
	
public:
	Game();
	Game(const Game& game);
	Piece* board[8][8] = { 0 };
	Piece* whitePieces[16] = { 0 };
	Piece* blackPieces[16] = { 0 };
	King* whiteKing;
	King* blackKing;
	bool turn;
	std::vector<move_info> move_log;

	//Methods
	void initialize_board();
	std::string switch_turns();
	int check_for_winner(bool color);
	bool check_for_check(bool color, position type = {-1, -1});
	move_info try_move(Piece* piece, position location);
	void move(Piece* piece, position location);
	void move_back(move_info log);
	void capture(Piece* piece);
	bool leap_then_look(Piece* piece, position move, King* king);
	bool log_move(Piece* piece, position move);
	//std::vector<position> get_moves(Piece*);
	std::vector<move_info> get_all_moves(bool color);
	void undo();
	void print_board();
};

