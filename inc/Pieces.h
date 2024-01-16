#pragma once
#include <vector>
#include <string>

class Piece;
class Game;

struct Position
{
	int x, y;

	bool operator==(const Position &other)
	{
		return (x == other.x && y == other.y);
	}
};

#define PAWN 0
#define NIGHT 1
#define BISHOP 2
#define ROOK 3
#define QUEEN 4
#define KING 5

struct Move
{
	Position from;
	Position to;

	bool operator==(const Move &other)
	{
		return (from == other.from && to == other.to);
	}

	std::string to_string()
	{
		return std::string(1, (char)from.x + 97) + std::to_string(8 - from.y) + std::string(1, (char)to.x + 97) + std::to_string(8 - to.y);
	}

	Move translate(std::string repr)
	{
		from.x = repr.c_str()[0] - 97;
		from.y = int(7 - (repr.c_str()[1] - 49));
		to.x = repr.c_str()[2] - 97;
		to.y = int(7 - (repr.c_str()[3] - 49));
		return *this;
	}
};

struct MoveDetails
{
	Move move;
	Piece *captured;
	bool promotion;
	char since_capture;
	char can_castle;
};

bool check_ob(Position move);

class Piece
{
public:
	unsigned char type;
	unsigned char x, y;
	char white;

	Piece();
	Piece(unsigned char pc, bool wht, unsigned char hor, unsigned char ver);

	bool can_capture(Piece *other);
	int find_valid_moves(Game &game, Position (&moves)[27]);
	char to_string()
	{
		char codes[12] = {0b1001, 0b1010, 0b1011, 0b1100, 0b1101, 0b1110, 0b0001, 0b0010, 0b0011, 0b0100, 0b0101, 0b0110};
		char buff = -1;
		if (white >= 0)
		{
			if (x & 1)
			{
				buff ^= codes[type + (white * 6)];
			}
			else
			{
				buff ^= codes[type + (white * 6)] << 4;
			}
		}
		return buff;
	}
};
