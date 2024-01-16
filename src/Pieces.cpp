#include "Pieces.h"
#include "Game.h"

using namespace std;

bool check_ob(Position move)
{
	if ((move.x < 0) || (move.x > 7) ||
		(move.y < 0) || (move.y > 7))
	{
		return true;
	}
	return false;
}

Piece::Piece()
{
	x = y = 0;
	white = true;
	type = PAWN;
}

Piece::Piece(unsigned char tp, bool wht, unsigned char hor, unsigned char ver)
{
	type = tp;
	white = wht;
	x = hor;
	y = ver;
}

bool Piece::can_capture(Piece *other)
{
	if (other == NULL)
	{
		return false;
	}
	if (other->white == white)
	{
		return false;
	}
	if (other->type == KING)
	{
		return false;
	}
	return true;
}

int Piece::find_valid_moves(Game &game, Position (&moves)[27])
{
	int num_moves = 0;

	if (type == PAWN)
	{
		int direction = white * (-2) + 1;
		// Basic Move
		if (!check_ob({x, y + direction}) && game.board[y + direction][x] == NULL)
		{
			if (!game.leap_then_look(this, {x, y + direction}))
			{

				moves[num_moves] = {x, y + direction};
				num_moves++;
				// Move two
			}
			if (!check_ob({x, y + 2 * direction}) && game.board[y + 2 * direction][x] == NULL && ((white && y == 6) || (!white && y == 1)) && !game.leap_then_look(this, {x, y + 2 * direction}))
			{
				moves[num_moves] = {x, y + 2 * direction};
				num_moves++;
			}
		}

		// Diagonal
		for (Position move : {Position{x + 1, y + direction}, Position{x - 1, y + direction}})
		{
			if (!check_ob(move))
			{
				if (can_capture(game.board[move.y][move.x]) && !game.leap_then_look(this, move))
				{

					moves[num_moves] = move;
					num_moves++;
				}
				// En passant
				else if (can_capture(game.board[y][move.x]) && game.board[y][move.x]->type == PAWN)
				{
					Move last_move;
					last_move.translate(game.move_log.back());
					if (last_move.to == Position{move.x, y} && game.board[last_move.to.y][last_move.to.x]->type == PAWN && abs(last_move.from.y - last_move.to.y) > 1 && !game.leap_then_look(this, move))
					{

						moves[num_moves] = move;
						num_moves++;
					}
				}
			}
		}
	}
	else if (type == NIGHT)
	{
		Position possibilities[] = {{x + 2, y - 1}, {x + 2, y + 1}, {x + 1, y + 2}, {x - 1, y + 2}, {x - 2, y + 1}, {x - 2, y - 1}, {x - 1, y - 2}, {x + 1, y - 2}};
		for (int i = 0; i < 8; i++)
		{
			Position move = possibilities[i];
			if (!check_ob(move) && (game.board[move.y][move.x] == NULL || can_capture(game.board[move.y][move.x])) && !game.leap_then_look(this, move))
			{

				moves[num_moves] = move;
				num_moves++;
			}
		}
	}
	else if (type == BISHOP)
	{
		Position dir[4] = {{1, 1}, {-1, 1}, {-1, -1}, {1, -1}};
		for (int i = 0; i < 4; i++)
		{
			int potX = x + dir[i].x;
			int potY = y + dir[i].y;
			while (potX > -1 && potY > -1 && potX < 8 && potY < 8)
			{
				if (game.board[potY][potX] == NULL)
				{
					if (!game.leap_then_look(this, {potX, potY}))
					{
						moves[num_moves] = {potX, potY};
						num_moves++;
					}
					potX += dir[i].x;
					potY += dir[i].y;
				}
				else
				{
					if (can_capture(game.board[potY][potX]) && !game.leap_then_look(this, {potX, potY}))
					{

						moves[num_moves] = {potX, potY};
						num_moves++;
					}
					break;
				}
			}
		}
	}
	else if (type == ROOK)
	{
		Position dir[4] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
		for (int i = 0; i < 4; i++)
		{
			int potX = x + dir[i].x;
			int potY = y + dir[i].y;
			while (potX > -1 && potY > -1 && potX < 8 && potY < 8)
			{
				if (game.board[potY][potX] == NULL)
				{
					if (!game.leap_then_look(this, {potX, potY}))
					{

						moves[num_moves] = {potX, potY};
						num_moves++;
					}
					potX += dir[i].x;
					potY += dir[i].y;
				}
				else
				{
					if (can_capture(game.board[potY][potX]) && !game.leap_then_look(this, {potX, potY}))
					{

						moves[num_moves] = {potX, potY};
						num_moves++;
					}
					break;
				}
			}
		}
	}
	else if (type == QUEEN)
	{
		Position dir[8] = {{1, 1}, {-1, 1}, {-1, -1}, {1, -1}, {1, 0}, {-1, 0}, {0, 1}, {0, -1}};
		for (int i = 0; i < 8; i++)
		{
			int potX = x + dir[i].x;
			int potY = y + dir[i].y;
			while (potX > -1 && potY > -1 && potX < 8 && potY < 8)
			{
				if (game.board[potY][potX] == NULL)
				{
					if (!game.leap_then_look(this, {potX, potY}))
					{

						moves[num_moves] = {potX, potY};
						num_moves++;
					}
					potX += dir[i].x;
					potY += dir[i].y;
				}
				else
				{
					if (can_capture(game.board[potY][potX]) && !game.leap_then_look(this, {potX, potY}))
					{

						moves[num_moves] = {potX, potY};
						num_moves++;
					}
					break;
				}
			}
		}
	}
	else
	{
		Position possibilities[8] = {{x + 1, y + 1}, {x + 1, y}, {x + 1, y - 1}, {x, y + 1}, {x, y - 1}, {x - 1, y + 1}, {x - 1, y}, {x - 1, y - 1}};
		for (Position move : possibilities)
		{
			if (!check_ob(move) && (game.board[move.y][move.x] == NULL || can_capture(game.board[move.y][move.x])) && !game.leap_then_look(this, move))
			{

				moves[num_moves] = move;
				num_moves++;
			}
		}

		// Castling
		if (!game.check_for_check(white, {x, y}))
		{
			// King side
			if (game.board[y][7] != NULL && game.board[y][7]->type == ROOK &&
				((white && game.can_castle & 0b1000) || (!white && game.can_castle & 0b0010)) &&
				game.board[y][5] == NULL && game.board[y][6] == NULL &&
				!game.leap_then_look(this, {5, y}) && !game.leap_then_look(this, {6, y}))
			{
				moves[num_moves] = {6, y};
				num_moves++;
			}
			// Queen side
			if (game.board[y][0] != NULL && game.board[y][0]->type == ROOK &&
				((white && game.can_castle & 0b0100) || (!white && game.can_castle & 0b0001)) &&
				game.board[y][1] == NULL && game.board[y][2] == NULL && game.board[y][3] == NULL && !game.leap_then_look(this, {3, y}) && !game.leap_then_look(this, {2, y}))
			{
				moves[num_moves] = {2, y};
				num_moves++;
			}
		}
	}
	return num_moves;
}
