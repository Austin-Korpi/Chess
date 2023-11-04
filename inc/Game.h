#pragma once
#include "Pieces.h"
#include <vector>
#include <array>
#include <string>

class Game {
	
public:
	Piece* board[8][8] = { 0 };
	std::array<Piece, 16> whitePieces;
	std::array<Piece, 16> blackPieces;
	Position whiteKing;
	Position blackKing;
	bool turn;
	bool castleWK;
	bool castleWQ;
	bool castleBK;
	bool castleBQ;
	int sinceCapture;
	std::vector<move_info> moveLog;

	Game();
	Game(const Game& game);
	Game& operator=(const Game& original);

	//Methods
	void initialize_board();
	std::string switch_turns();
	int check_for_winner(bool color);
	bool check_for_check(bool color, Position type = {-1, -1});
	move_info move(Piece* piece, Position location);
	void movePiece(Piece* piece, Position location);
	void capture(Piece* piece);
	bool leap_then_look(Piece* piece, Position move);
	bool log_move(move_info move);
	//std::vector<position> get_moves(Piece*);
	move_info get_random_move(bool color);
	std::vector<move_info> get_all_moves(bool color);
	void print_board();
};

