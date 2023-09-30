#pragma once
#include <vector>

class Piece;
class Game;

struct position{
	int x, y;

	bool operator==(const position& other) {
		return(x == other.x && y == other.y);
	}
};

typedef enum {
	pawn = 0,
	night = 1,
	bishop = 2,
	rook = 3,
	queen = 4,
	king = 5,
	all
} pieceOptions;


struct move_info {
	position from;
	position to;
	Piece* piece;
	Piece* captured;
	bool check;

	bool operator==(const move_info & other) {
		return(from == other.from && to == other.to/* && piece == other.piece && captured = other.captured*/);
	}
};


bool check_ob(position move);
bool has_moved(Piece* piece, std::vector<move_info> &move_log);

class Piece {
public:
	int id;
	pieceOptions piece;
	bool white;
	int x, y;

	Piece(int i, pieceOptions pc, bool wht, int hor, int ver);

	bool can_capture(Piece* other);
	virtual int find_valid_moves(Game &game, position (&moves)[27]) = 0;
};


class Pawn : public Piece {
public:
	Pawn(int i, bool wht, int hor, int ver);
	int find_valid_moves(Game &game, position (&moves)[27]);
};

class Night : public Piece {
public:
	Night(int i, bool wht, int hor, int ver);
	int find_valid_moves(Game &game, position (&moves)[27]);
};

class Bishop : public Piece {
public:
	Bishop(int i, bool wht, int hor, int ver);
	int find_valid_moves(Game &game, position (&moves)[27]);
};

class Rook : public Piece {
public:
	Rook(int i, bool wht, int hor, int ver);
	int find_valid_moves(Game &game, position (&moves)[27]);
};

class Queen : public Piece {
public:
	Queen(int i, bool wht, int hor, int ver);
	int find_valid_moves(Game &game, position (&moves)[27]);
};

class King : public Piece {
public:
	King(int i, bool wht, int hor, int ver);
	int find_valid_moves(Game &game, position (&moves)[27]);
};
