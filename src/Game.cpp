#include "Game.h"
#include <iostream>
#include <string>
#include <cstring>

Game::Game()
{
    moveLog = std::vector<move_info>();
    turn = true;
    castleK = true;
	castleQ = true;
	castlek = true;
	castleq = true;
	sinceCapture = 0;

    // Initialize pieces
    whitePieces[8] = Piece(8, rook, true, 0, 7);
    whitePieces[9] = Piece(9, night, true, 1, 7);
    whitePieces[10] = Piece(10, bishop, true, 2, 7);
    whitePieces[11] = Piece(11, queen, true, 3, 7);
    whitePieces[12] = Piece(12, king, true, 4, 7);
    whitePieces[13] = Piece(13, bishop, true, 5, 7);
    whitePieces[14] = Piece(14, night, true, 6, 7);
    whitePieces[15] = Piece(15, rook, true, 7, 7);
    whitePieces[0] = Piece(0, pawn, true, 0, 6);
    whitePieces[1] = Piece(1, pawn, true, 1, 6);
    whitePieces[2] = Piece(2, pawn, true, 2, 6);
    whitePieces[3] = Piece(3, pawn, true, 3, 6);
    whitePieces[4] = Piece(4, pawn, true, 4, 6);
    whitePieces[5] = Piece(5, pawn, true, 5, 6);
    whitePieces[6] = Piece(6, pawn, true, 6, 6);
    whitePieces[7] = Piece(7, pawn, true, 7, 6);

    blackPieces[0] = Piece(0, rook, false, 0, 0);
    blackPieces[1] = Piece(1, night, false, 1, 0);
    blackPieces[2] = Piece(2, bishop, false, 2, 0);
    blackPieces[3] = Piece(3, queen, false, 3, 0);
    blackPieces[4] = Piece(4, king, false, 4, 0);
    blackPieces[5] = Piece(5, bishop, false, 5, 0);
    blackPieces[6] = Piece(6, night, false, 6, 0);
    blackPieces[7] = Piece(7, rook, false, 7, 0);
    blackPieces[8] = Piece(8, pawn, false, 0, 1);
    blackPieces[9] = Piece(9, pawn, false, 1, 1);
    blackPieces[10] = Piece(10, pawn, false, 2, 1);
    blackPieces[11] = Piece(11, pawn, false, 3, 1);
    blackPieces[12] = Piece(12, pawn, false, 4, 1);
    blackPieces[13] = Piece(13, pawn, false, 5, 1);
    blackPieces[14] = Piece(14, pawn, false, 6, 1);
    blackPieces[15] = Piece(15, pawn, false, 7, 1);

    // Save the location of the kings
    whiteKing = {whitePieces[12].x, whitePieces[12].y};
    blackKing = {blackPieces[4].x, blackPieces[4].y};

    // Place the pieces on the board
    initialize_board();
}

Game::Game(const Game &game) : whitePieces(game.whitePieces),
                               blackPieces(game.blackPieces),
                               whiteKing(game.whiteKing),
                               blackKing(game.blackKing),
                               turn(game.turn),
                               castleK(game.castleK),
                               castleQ(game.castleQ),
                               castlek(game.castlek),
                               castleq(game.castleq),
                               sinceCapture(game.sinceCapture),
                               moveLog(game.moveLog)
{
    // Place the pieces on the board
    initialize_board();
}

Game& Game::operator=(const Game& original) {
    if (this != &original) {
        whitePieces = original.whitePieces;
        blackPieces = original.blackPieces;
        whiteKing = original.whiteKing;
        blackKing = original.blackKing;
        moveLog = original.moveLog;
        turn = original.turn;
        castleK = original.castleK;
        castleQ = original.castleQ;
        castlek = original.castlek;
        castleq = original.castleq;
        sinceCapture = original.sinceCapture;
        
        initialize_board();
    }
    return *this;
}

void Game::initialize_board()
{
    // Initialize 8 by 8 board of empty pieces
    memset(board, 0, sizeof(void*) * 64);

    // Place the white pieces
    for (int i = 0; i < 16; i++)
    {
        Piece* piece = &whitePieces[i];
        if (!piece->captured)
        {
            int x = piece->x;
            int y = piece->y;
            board[y][x] = piece;
        }
    }
    // Place the black pieces
    for (int i = 0; i < 16; i++)
    {
        Piece* piece = &blackPieces[i];
        if (!piece->captured)
        {
            int x = piece->x;
            int y = piece->y;
            board[y][x] = piece;
        }
    }
}

std::string Game::switch_turns()
{
    // Check if the game is over
    int winner;
    winner = check_for_winner(turn);

    // Switch the turn
    turn = !turn;

    // Return a string with the winner
    if (winner == 1)
    {
        return turn ? "Black Wins!" : "White Wins!";
    }
    else if (winner == -1)
    {
        return "Stalemate";
    }
    return "";
}

int Game::check_for_winner(bool color)
{
    // Check for stalemate
    if (moveLog.size() > 9)
    {
        size_t n = moveLog.size();
        if (moveLog[n - 1] == moveLog[n - 5] &&
            moveLog[n - 1] == moveLog[n - 9] &&
            moveLog[n - 2] == moveLog[n - 6] &&
            moveLog[n - 3] == moveLog[n - 7] &&
            moveLog[n - 4] == moveLog[n - 8])
        {
            return -1;
        }
    }

    if (sinceCapture >= 100) {
        return -1;
        printf("50 move rule\n");
    }

    auto pieces = color ? blackPieces : whitePieces;
    for (int i = 0; i < 16; i++)
    {
        if (!pieces[i].captured)
        {
            Piece piece = pieces[i];
            Position moves[27];
            int n = piece.find_valid_moves(*this, moves);
            for (int j = 0; j < n; j++)
            {
                if (!leap_then_look(&piece, moves[j]))
                    return 0;
            }
        }
    }

    if (check_for_check(!color))
        return 1;

    return -1;
}

bool Game::check_for_check(bool color, Position location)
{
    // Default location
    if (location == Position{-1, -1})
    {
        location = color ? whiteKing : blackKing;
    }
    {
        Position dir[4] = {{1, 1}, {-1, 1}, {-1, -1}, {1, -1}};
        for (int k = 0; k < 4; k++)
        {
            int i = location.x + dir[k].x;
            int j = location.y + dir[k].y;
            while (i > -1 && j > -1 && i < 8 && j < 8)
            {
                Piece *piece = board[j][i];

                if (piece == NULL)
                {
                    i += dir[k].x;
                    j += dir[k].y;
                }
                else
                {
                    if (piece->white != color && (piece->type == bishop || piece->type == queen))
                    {
                        return true;
                    }
                    break;
                }
            }
        }
    }

    Position dir[4] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

    for (int k = 0; k < 4; k++)
    {
        int i = location.x + dir[k].x;
        int j = location.y + dir[k].y;
        while (i > -1 && j > -1 && i < 8 && j < 8)
        {
            Piece *piece = board[j][i];
            if (piece == NULL)
            {
                i += dir[k].x;
                j += dir[k].y;
            }
            else
            {
                if (piece->white != color && (piece->type == rook || piece->type == queen))
                {
                    return true;
                }
                break;
            }
        }
    }
    int x = location.x;
    int y = location.y;
    Position possibilities[] = {{x + 2, y - 1}, {x + 2, y + 1}, {x + 1, y + 2}, {x - 1, y + 2}, {x - 2, y + 1}, {x - 2, y - 1}, {x - 1, y - 2}, {x + 1, y - 2}};
    for (int i = 0; i < 8; i++)
    {
        Position move = possibilities[i];
        if (!check_ob(move))
        {
            Piece *piece = board[move.y][move.x];
            if (piece && piece->white == !color && piece->type == night)
            {
                return true;
            }
        }
    }

    Position enemyKing = !color ? whiteKing : blackKing;
    if (abs(enemyKing.y - y) < 2 && abs(enemyKing.x - x) < 2)
    {
        return true;
    }

    int direction = !color * -2 + 1;
    if (y - direction < 8 && y - direction > -1)
    {
        if (x < 7)
        {
            Piece *piece = board[y - direction][x + 1];
            if (piece && piece->white != color && piece->type == pawn)
                return true;
        }
        if (x > 0)
        {
            Piece *piece = board[y - direction][x - 1];
            if (piece && piece->white != color && piece->type == pawn)
                return true;
        }
    }

    return false;
}

move_info Game::move(Piece *piece, Position location)
{
    move_info log = {{piece->x, piece->y}, location}; //, piece, NULL, false };
    
    sinceCapture++;

    // Capture
    if (board[location.y][location.x] != NULL)
    {
        capture(board[location.y][location.x]);
        sinceCapture = 0;
    }
    else if (piece->type == pawn && piece->x != location.x)
    {
        capture(board[piece->y][location.x]);
        sinceCapture = 0;
    }

    // Castling
    if (piece->type == king && abs(location.x - piece->x) > 1)
    {
        if (location.x < piece->x)
        {
            movePiece(board[piece->y][0], {3, piece->y});
        }
        else
        {
            movePiece(board[piece->y][7], {5, piece->y});
        }
    }

    // Promotion
    if (piece->type == pawn && location.y == (piece->white ? 0 : 7))
    {
        piece->type = queen;
    }

    movePiece(piece, location);

    if (piece->type == pawn)
        sinceCapture = 0;

    if (piece->type == king && piece->white)
    {
        whiteKing = {piece->x, piece->y};
        castleK = false;
        castleQ = false;
    }
    if (piece->type == king && !piece->white)
    {
        blackKing = {piece->x, piece->y};
        castlek = false;
        castleq = false;
    }

    if (piece->type == rook && piece->white && log.from.x == 0)
        castleQ = false;

    if (piece->type == rook && piece->white && log.from.x == 7)
        castleK = false;

    if (piece->type == rook && !piece->white && log.from.x == 0)
        castleq = false;

    if (piece->type == rook && !piece->white && log.from.x == 7)
        castlek = false;

    return log;
}

void Game::movePiece(Piece *piece, Position location)
{
    board[piece->y][piece->x] = NULL;
    board[location.y][location.x] = piece;
    piece->x = location.x;
    piece->y = location.y;
}

void Game::capture(Piece *piece)
{
    piece->captured = true;
    board[piece->y][piece->x] = NULL;
}

bool Game::leap_then_look(Piece *piece, Position move)
{
    Game test(*this);
    test.move(test.board[piece->y][piece->x], move);
    Position king = piece->white ? test.whiteKing : test.blackKing;
    return test.check_for_check(test.board[king.y][king.x]->white, king);
}

bool Game::log_move(move_info move)
{
    Position moves[27];
    // Check if move is valid
    Piece *piece = board[move.from.y][move.from.x];
    int n = piece->find_valid_moves(*this, moves);
    bool invalid = true;
    for (int i = 0; i < n; i++)
    {
        if (move.to == moves[i])
        {
            invalid = false;
            break;
        }
    }
    if (invalid)
    {
        return false;
    }

    // Move piece
    move_info log = this->move(piece, move.to);

    // Add move to log
    moveLog.push_back(log);

    return true;
}

move_info Game::get_random_move(bool color)
{
    auto pieces = color ? whitePieces : blackPieces;
    Piece piece;
    int n = 0;
    Position pieceMoves[27];
    while (n == 0)
    {
        piece = pieces[rand() % 16];
        if (!piece.captured)
        {
            n = piece.find_valid_moves(*this, pieceMoves);
        }
    }

    return move_info{{piece.x, piece.y}, pieceMoves[rand() % n]}; //, piece, NULL, false};
}

std::vector<move_info> Game::get_all_moves(bool color)
{
    std::vector<move_info> moves = std::vector<move_info>();
    auto pieces = color ? whitePieces : blackPieces;

    for (int i = 0; i < 16; i++)
    {
        if (!pieces[i].captured)
        {
            Piece piece = pieces[i];
            Position pieceMoves[27];
            int n = piece.find_valid_moves(*this, pieceMoves);
            for (int j = 0; j < n; j++)
            {
                moves.push_back(move_info{{piece.x, piece.y}, pieceMoves[j]}); //, piece, NULL, false});
            }
        }
    }
    return moves;
}

std::string Game::toString() {
    char repr[35];
    memset(repr, -1, 35);

    for (auto piece : whitePieces) {
        int i = (piece.y*8+piece.x) >> 1;
        repr[i] &= piece.toString();
    }

     for (auto piece : blackPieces) {
        int i = (piece.y*8+piece.x) >> 1;
        repr[i] &= piece.toString();
    }

    repr[32] &= turn;
    repr[32] &= castleK << 1;
    repr[32] &= castleQ << 2;
    repr[32] &= castlek << 3;
    repr[32] &= castleq << 4;
    int i = moveLog.size();
    if (i) {
        move_info lastMove = moveLog[i-1];
        if(board[lastMove.to.y][lastMove.to.x]->type == pawn && abs(lastMove.from.y - lastMove.to.y) > 1) {
            repr[32] &= 1 << 5;
            repr[33] &= lastMove.to.x;
            repr[33] &= (lastMove.to.y == 4) << 3;
        }
    }
    repr[34] = 0;

    // std::string repr = "";
    // char white[6] = {'P', 'N', 'B', 'R', 'Q', 'K'};
    // char black[6] = {'p', 'n', 'b', 'r', 'q', 'k'};
    // for (int i = 0; i < 8; i++)
    // {
    //     for (int j = 0; j < 8; j++)
    //     {
    //         Piece* piece = board[i][j];
    //         if (piece != NULL)
    //             repr += piece->white ? white[piece->type] : black[piece->type];
    //         else
    //              repr += "/";
    //     }
    // }
    // // Add extra state variables: castling rights, moves since capture, enPassant
    // repr += turn ? "W" : "B";
    // repr += castleK ? "K" : "";
    // repr += castleQ ? "Q" : "";
    // repr += castlek ? "k" : "";
    // repr += castleq ? "q" : "";
    // if (moveLog.size()) {
    //     move_info lastMove = moveLog.back();
    //     if(board[lastMove.to.y][lastMove.to.x]->type == pawn && abs(lastMove.from.y - lastMove.to.y) > 1) {
    //         repr += lastMove.toString();
    //     }
    // }
    return std::string(repr);
}

void Game::print_board()
{
    char letters[6] = {'P', 'N', 'B', 'R', 'Q', 'K'};
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            if (board[i][j] != NULL)
                std::cout << letters[board[i][j]->type] << " ";
            else
                std::cout << "_ ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";

    std::cout << "White Pieces: ";
    for (auto piece : whitePieces)
        if (!piece.captured)
            std::cout << letters[piece.type] << ", ";

    std::cout << "\n";

    std::cout << "Black Pieces: ";
    for (auto piece : blackPieces)
        if (!piece.captured)
            std::cout << letters[piece.type] << ", ";

    std::cout << "\n";
}