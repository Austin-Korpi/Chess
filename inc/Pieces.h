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


struct Move {
	Position from;
	Position to;

	bool operator==(const Move & other) {
		return(from == other.from && to == other.to);
	}

	std::string toString() {
		return std::string(1, (char) from.x+97) + std::to_string(8-from.y)+std::string(1, (char) to.x+97) +std::to_string(8-to.y);
	}
	
	Move translate(std::string repr) {
		from.x = repr.c_str()[0]-97;
		from.y = int(7 - (repr.c_str()[1] - 49));
		to.x = repr.c_str()[2]-97;
		to.y = int(7 - (repr.c_str()[3] - 49));		
		return *this;
	}
};

struct MoveDetails {
	Move move;
	Piece* captured;
	bool promotion;
	char sinceCapture;
	char canCastle;
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
	char toString() {
    	char codes[12] = {0b1001, 0b1010, 0b1011, 0b1100, 0b1101, 0b1110, 0b0001, 0b0010, 0b0011, 0b0100, 0b0101, 0b0110};
		char buff = -1;
		if (!captured) {
			if (x & 1) {
				buff ^= codes[type + (white * 6)];
			} else {
				buff ^= codes[type + (white * 6)] << 4;
			}
		}
		return buff;
	}
};
