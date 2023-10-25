#include "Game.h"
#include <iostream>



Game::Game() {
    turn = true;

    whitePieces[8 ] = new Rook  (8 , true, 0, 7);
    whitePieces[9 ] = new Night (9 , true, 1, 7);
    whitePieces[10] = new Bishop(10, true, 2, 7);
    whitePieces[11] = new Queen (11, true, 3, 7);
    whitePieces[12] = new King  (12, true, 4, 7);
    whitePieces[13] = new Bishop(13, true, 5, 7);
    whitePieces[14] = new Night (14, true, 6, 7);
    whitePieces[15] = new Rook  (15, true, 7, 7);
    whitePieces[0 ] = new Pawn  (0 , true, 0, 6);
    whitePieces[1 ] = new Pawn  (1 , true, 1, 6);
    whitePieces[2 ] = new Pawn  (2 , true, 2, 6);
    whitePieces[3 ] = new Pawn  (3 , true, 3, 6);
    whitePieces[4 ] = new Pawn  (4 , true, 4, 6);
    whitePieces[5 ] = new Pawn  (5 , true, 5, 6);
    whitePieces[6 ] = new Pawn  (6 , true, 6, 6);
    whitePieces[7 ] = new Pawn  (7 , true, 7, 6);
   
    whiteKing = (King*) whitePieces[12];

    blackPieces[0 ] = new Rook  (0 , false, 0, 0);
    blackPieces[1 ] = new Night (1 , false, 1, 0);
    blackPieces[2 ] = new Bishop(2 , false, 2, 0);
    blackPieces[3 ] = new Queen (3 , false, 3, 0);
    blackPieces[4 ] = new King  (4 , false, 4, 0);
    blackPieces[5 ] = new Bishop(5 , false, 5, 0);
    blackPieces[6 ] = new Night (6 , false, 6, 0);
    blackPieces[7 ] = new Rook  (7 , false, 7, 0);
    blackPieces[8 ] = new Pawn  (8 , false, 0, 1);
    blackPieces[9 ] = new Pawn  (9 , false, 1, 1);
    blackPieces[10] = new Pawn  (10, false, 2, 1);
    blackPieces[11] = new Pawn  (11, false, 3, 1);
    blackPieces[12] = new Pawn  (12, false, 4, 1);
    blackPieces[13] = new Pawn  (13, false, 5, 1);
    blackPieces[14] = new Pawn  (14, false, 6, 1);
    blackPieces[15] = new Pawn  (15, false, 7, 1);

    blackKing = (King*)blackPieces[4];

    initialize_board();
    move_log = std::vector<move_info>();

}

void replace_pieces(std::vector<move_info>& move_log, Piece* oldPiece, Piece* newPiece) {
    for (auto log : move_log) {
        if (log.piece == oldPiece)
            log.piece = newPiece;
        if (log.captured == oldPiece)
            log.captured = newPiece;
    }
}

Game::Game(const Game &game) {
    // printf("Duplicating a Game object\n");
    turn = game.turn;

    move_log = game.move_log;

    whiteKing = new King(*game.whiteKing);
    blackKing = new King(*game.blackKing);

    Piece* newPiece = NULL;
    for (auto piece : game.whitePieces) {
        if (piece == NULL)
            continue;
        switch (piece->piece) {
        case pawn:
            newPiece = new Pawn(*((Pawn*)piece));
            break;
        case night:
            newPiece = new Night(*((Night*)piece));
            break;
        case bishop:
            newPiece = new Bishop(*((Bishop*)piece));
            break;
        case rook:
            newPiece = new Rook(*((Rook*)piece));
            break;
        case queen:
            newPiece = new Queen(*((Queen*)piece));
            break;
        case king:
            newPiece = whiteKing;
            break;
        default:
            break;
        }
         
        whitePieces[newPiece->id] = newPiece;
        replace_pieces(move_log, piece, newPiece);
    }

    for (auto piece : game.blackPieces) {
        if (piece == NULL)
            continue;
        switch (piece->piece) {
        case pawn:
            newPiece = (new Pawn(*((Pawn*)piece)));
            break;
        case night:
            newPiece = (new Night(*((Night*)piece)));
            break;
        case bishop:
            newPiece = (new Bishop(*((Bishop*)piece)));
            break;
        case rook:
            newPiece = (new Rook(*((Rook*)piece)));
            break;
        case queen:
            newPiece = (new Queen(*((Queen*)piece)));
            break;
        case king:
            newPiece = blackKing;
            break;
        default:
            break;
        }
         
        blackPieces[newPiece->id] = newPiece;
        replace_pieces(move_log, piece, newPiece);

    }

    initialize_board();
}

Game::~Game() {
    for(int i = 0; i < 16; i++) {
        delete whitePieces[i];
    }
    for(int i = 0; i < 16; i++) {
        delete blackPieces[i];
    }
}

void Game::initialize_board() {
    //Initialize 8 by 8 board of empty pieces
    for (int i = 0; i < 8; i++ ) {
        for (int j = 0; j < 8; j++) {
            board[i][j] = NULL;
        }
    }

    for (Piece* piece : whitePieces) {
        if (piece) {
            int x = piece->x;
            int y = piece->y;
            board[y][x] = piece;
        }
    }
    for (Piece* piece : blackPieces) {
        if (piece) {
            int x = piece->x;
            int y = piece->y;
            board[y][x] = piece;
        }
    }
}

std::string Game::switch_turns() {
    int winner;
    winner = check_for_winner(turn);

    turn = !turn;

    if (winner == 1) {
        return turn ? "Black Wins!" : "White Wins!";
    } else if (winner == -1) {
        return "Stalemate";
    } 
    return "";
}

int Game::check_for_winner(bool color) {
    //Check for stalemate
    if (move_log.size() > 9) {
        size_t n = move_log.size();
        if (move_log[n - 1] == move_log[n - 5] &&
            move_log[n - 1] == move_log[n - 9] &&
            move_log[n - 2] == move_log[n - 6] &&
            move_log[n - 3] == move_log[n - 7] &&
            move_log[n - 4] == move_log[n - 8])
        {
            // return "Stalemate";
            return -1;
        }
    }

    if (move_log.size() >= 50) {
        bool capture = false;
        for (unsigned int i = move_log.size() - 50; i < move_log.size(); i++) {
            if (move_log[i].captured != NULL) {
                capture = true;
                break;
            }
        }
        if (!capture) {
            // return "Stalemate";
            return -1;
        }
    }

    King* curKing = color ? blackKing : whiteKing; 
    Piece** pieces = color ? blackPieces : whitePieces;
    for(int i = 0; i < 16; i++){
        if (pieces[i] != NULL) {
            Piece* piece = pieces[i];
            position moves[27];
            int n = piece->find_valid_moves(*this, moves);
            for (int j = 0; j < n; j++) {
                if (!leap_then_look(piece, moves[j], curKing)) {
                    // return "";
                    return 0;
                }
            }
        }
    }

    if (check_for_check(!color)) {
        return 1;
    //     return color ? "White Wins!" : "Black Wins!";
    }

    // return "Stalemate";
    return -1;
}

  
bool Game::check_for_check(bool color, position location) {
    //Default location
    if (location == position{-1, -1}){
        King king = color ? *whiteKing : *blackKing;
        location = { king.x, king.y };
    }
    {
        position dir[4] = { {1,1},{-1,1},{-1,-1},{1,-1} };
        for (int k = 0; k < 4; k++) {
            int i = location.x + dir[k].x;
            int j = location.y + dir[k].y;
            while (i > -1 && j > -1 && i < 8 && j < 8) {
                Piece* piece = board[j][i];

                if (piece == NULL) {
                    i += dir[k].x;
                    j += dir[k].y;
                }
                else {
                    if (piece->white != color && (piece->piece == bishop || piece->piece == queen)) {
                        return true;
                    }
                    break;
                }
            }
        }
    }
    
    position dir[4] = {{1,0},{-1,0},{0,1},{0,-1}};
    
    for (int k = 0; k < 4; k++) {
        int i = location.x + dir[k].x;
        int j = location.y + dir[k].y;
        while (i > -1 && j > -1 && i < 8 && j < 8) {
            Piece* piece = board[j][i];
            if (piece == NULL) {
                i += dir[k].x;
                j += dir[k].y;
            }
            else {
                if (piece->white != color && (piece->piece == rook || piece->piece == queen)) {
                    return true;
                }
                break;
            }
        }
    }
    int x = location.x;
    int y = location.y;
    position possibilities[] = { {x + 2, y - 1}, {x + 2, y + 1},
        {x + 1, y + 2}, {x - 1, y + 2}, {x - 2, y + 1},
        {x - 2, y - 1}, {x - 1, y - 2}, {x + 1, y - 2} };
    for (int i = 0; i < 8; i++) {
        position move = possibilities[i];
        if (!check_ob(move)) {
            Piece* piece = board[move.y][move.x];
            if (piece && piece->white == !color && piece->piece == night) {
                return true;
            }
        }
    }

    King* enemyKing = !color ? whiteKing : blackKing;
    if (abs(enemyKing->y - y) < 2 && abs(enemyKing->x - x) < 2) {
        return true;
    }

    int direction = !color * -2 + 1;
    if (y - direction < 8 && y - direction > -1) {
        if (x < 7) {
            Piece* piece = board[y - direction][x + 1];
            if (piece && piece->white != color && piece->piece == pawn)
                return true;
        }
        if (x > 0) {
            Piece* piece = board[y - direction][x - 1];
            if (piece && piece->white != color && piece->piece == pawn)
                return true;
        }
    }
    /*for (position move : {position{ x + 1 ,y - direction }, position{ x - 1 ,y - direction } }) {
        if (!check_ob(move)) {
            Piece* piece = board[move.y][move.x];
            if (piece && piece->piece == pawn && piece->white != color) {
                return true;
            }
        }
    }*/

    return false;
}

move_info Game::try_move(Piece* piece, position location) {
    move_info log = { {piece->x, piece->y }, location, piece, NULL, false };

    if (board[location.y][location.x] != NULL) {
        log.captured = board[location.y][location.x];
        capture(board[location.y][location.x]);
    }
    else if (piece->piece == pawn && piece->x != location.x) {
        log.captured = board[piece->y][location.x];
        capture(board[piece->y][location.x]);
    }

    if (piece->piece == king && abs(location.x - piece->x) > 1) {
        if (location.x < piece->x) {
            move(board[piece->y][0], { 3, piece->y});
        }
        else {
            move(board[piece->y][7], { 5, piece->y });
        }
    }

    if (piece->piece == pawn && location.y == (piece->white ? 0 : 7)) {
        Piece* newPiece = new Queen(piece->id, piece->white, piece->x, piece->y);
        Piece** pieces = piece->white ? whitePieces : blackPieces; 
        pieces[piece->id] = newPiece;
        piece = newPiece;
    }

    move(piece, location);

    return log;
}

void Game::move(Piece* piece, position location) {
    board[piece->y][piece->x] = NULL;
    board[location.y][location.x] = piece;
    piece->x = location.x;
    piece->y = location.y;
}

void Game::capture(Piece* piece) { 
    Piece** pieces = piece->white ? whitePieces : blackPieces;
    pieces[piece->id] = NULL;

    board[piece->y][piece->x] = NULL;
}

void Game::move_back(move_info log) {

    //Undo promotion
    if (log.piece != board[log.to.y][log.to.x]) { 
        Piece** pieces = log.piece->white ? whitePieces : blackPieces;
        pieces[log.piece->id] = log.piece;

        board[log.to.y][log.to.x] = NULL;
    }
    //Move piece back
    move(log.piece, log.from);

    //Undo castling
    if (log.piece->piece == king && abs(log.from.x - log.to.x) > 1) {
        if (log.to.x < 3) {
            move(board[log.piece->y][3], { 0, log.piece->y });
        }
        else {
            move(board[log.piece->y][5], { 7, log.piece->y });
        }
    }
    //Return captured piece
    if (log.captured != NULL) {
        move(log.captured, { log.captured->x, log.captured->y });
        Piece** pieces = log.captured->white ? whitePieces : blackPieces;
        pieces[log.captured->id] = log.captured;

    }
}

bool Game::leap_then_look(Piece* piece, position move, King* king) {
    move_info log = try_move(piece, move);
    bool check = check_for_check(king->white, { king->x, king->y });
    move_back(log);
    return check;
}

bool Game::log_move(Piece* piece, position move) {
    position moves[27];
    int n = piece->find_valid_moves(*this, moves);
    bool invalid = true;
    for(int i = 0; i < n; i++){
        if(move == moves[i]){
            invalid = false;
            break;
        }
    }
    if (invalid) {
        return false;
    }

    move_info log = try_move(piece, move);
    log.check = check_for_check(!turn);

    move_log.push_back(log);

    return true;

}

move_info Game::get_random_move(bool color) {
    Piece** pieces = color ? whitePieces : blackPieces;
    Piece* piece;
    int n = 0;
    position pieceMoves[27];
    while (n == 0) {
        piece = pieces[rand()%16];
        if (piece) {
            n = piece->find_valid_moves(*this, pieceMoves);
        }
    }
    
    return move_info{ {piece->x, piece->y }, pieceMoves[rand()%n], piece, NULL, false};
}

std::vector<move_info> Game::get_all_moves(bool color) {
    std::vector<move_info> moves = std::vector<move_info>();
    Piece** pieces = color ? whitePieces : blackPieces;

    for(int i = 0; i < 16; i++){
        if (pieces[i] != NULL) {
            Piece* piece = pieces[i];
            position pieceMoves[27];
            int n = piece->find_valid_moves(*this, pieceMoves);
            for(int j = 0; j < n; j++){
                moves.push_back(move_info{ {piece->x, piece->y }, pieceMoves[j], piece, NULL, false});
            }
        }
    }
    return moves;
}

void Game::undo() {
    if (move_log.size() == 0) {
        return;
    }
    move_info log = move_log.back();
    move_log.pop_back();
    move_back(log);
    turn = !turn;
}

void Game::print_board() {
    char letters[6] = { 'P', 'N', 'B', 'R', 'Q', 'K' };
    for(int i = 0; i < 8; i++){
        for(int j=0;j<8;j++){
            if (board[i][j] != NULL)
                std::cout << letters[board[i][j]->piece] << " ";
            else
                std::cout << "_ ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";

    std::cout << "White Pieces: ";
    for (auto piece : whitePieces)
        if (piece != NULL)
            std::cout << letters[piece->piece] << ", ";

    std::cout << "\n";

    std::cout << "Black Pieces: ";
    for (auto piece : blackPieces)
        if (piece != NULL)
            std::cout << letters[piece->piece] << ", ";
    
    std::cout << "\n";

}