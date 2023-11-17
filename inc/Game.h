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
	char canCastle;
	// bool castleK;
	// bool castleQ;
	// bool castlek;
	// bool castleq;
	char sinceCapture;
	std::vector<Move> moveLog;

	Game();
	Game(const Game& game);
	Game& operator=(const Game& original);

	//Methods
	void initialize_board();
	std::string switch_turns();
	int check_for_winner(bool color);
	bool check_for_check(bool color, Position type = {-1, -1});
	MoveDetails move(Piece* piece, Position location);
	void moveBack(MoveDetails);
	void movePiece(Piece* piece, Position location);
	void capture(Piece* piece);
	bool leap_then_look(Piece* piece, Position move);
	bool log_move(Move move);
	//std::vector<position> get_moves(Piece*);
	Move get_random_move(bool color);
	std::vector<Move> get_all_moves(bool color);
	void print_board();
	std::string toString();
};

