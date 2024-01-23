#ifndef GAME_H
#define GAME_H

#include "Pieces.h"
#include <vector>
#include <array>
#include <string>

class Game
{

public:
	std::array<Piece, 16> white_pieces;
	std::array<Piece, 16> black_pieces;
	bool turn;
	char can_castle;
	char since_capture;
	Position white_king;
	Position black_king;
	Piece *board[8][8] = {0};
	std::vector<std::string> move_log;
	char has_castled;

	Game();
	Game(const Game &game);
	Game &operator=(const Game &original);

	// Methods
	void initialize_board();
	std::string switch_turns();
	int check_for_winner(bool color);
	bool check_for_check(bool color, Position type = {-1, -1});
	MoveDetails move(Piece *piece, Position location);
	void add_to_log(MoveDetails);
	void move_back(MoveDetails);
	void move_piece(Piece *piece, Position location);
	void capture(Piece *piece);
	bool leap_then_look(Piece *piece, Position move);
	bool log_move(Move move);
	Move get_random_move(bool color);
	std::vector<Move> get_all_moves(bool color);
	int count_doubled_pawns(bool color);
	int count_blocked_pawns(bool color);
	int count_isolated_pawns(bool color);
	int count_bid_pawns(bool color);
	int count_passed_pawns(bool color);
	int count_rook_attacks(bool color);
	void print_board();
	std::string to_string();
};

#endif