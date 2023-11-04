#pragma once
#include <vector>
#include <string> 

class Piece;
class Game;

struct Position{
	int x, y;

	bool operator==(const Position& other) {
		return(x == other.x && y == other.y);
	}
};

typedef enum {
	pawn = 0,
	night = 1,
	bishop = 2,
	rook = 3,
	queen = 4,
	king = 5
} pieceOptions;


struct move_info {
	Position from;
	Position to;
	// Piece* piece;
	// Piece* captured;
	// bool check;

	bool operator==(const move_info & other) {
		return(from == other.from && to == other.to/* && piece == other.piece && captured = other.captured*/);
	}

	std::string toString() {
		return std::to_string(from.x) + std::to_string(from.y)+" "+std::to_string(to.x) +std::to_string(to.y);
	}
};


bool check_ob(Position move);

class Piece {
public:
	int id;
	pieceOptions type;
	int x, y;
	// Position position;
	bool white;
	bool captured;

	Piece();
	Piece(int i, pieceOptions pc, bool wht, int hor, int ver);

	bool can_capture(Piece* other);
	int find_valid_moves(Game &game, Position (&moves)[27]);
};
