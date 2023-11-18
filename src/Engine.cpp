#include <thread>
#include <mutex>
#include <iostream>
#include <climits>
#include <cstring>
#include <unordered_map>
#include <algorithm>
#include "ctpl_stl.h"

#include "Engine.h"
#include "Opening.h"
#include "oneapi/tbb/concurrent_hash_map.h"

int maxdepth = MAXDEPTH;

// Global Data collection variables
int visited[MAXDEPTH+1] = {0};

typedef oneapi::tbb::concurrent_hash_map<std::string, std::array<int, 3>> StringTable;
StringTable transpositionTable;
std::mutex tableLock;
bool useTransposition = false;

int heuristic(Game &game) {
	int material = 0;
	// Lookup table
	int worth[] = { 1,3,3,5,9,0 };

	// White pieces
	for (auto piece : game.whitePieces) {
		if (piece.white >= 0) {
			material += worth[piece.type];
		}
	}
	// Black pieces
	for (auto piece : game.blackPieces) {
		if (piece.white >= 0) {
			material -= worth[piece.type];
		}
	}
	return material;
}

int utility(int status, bool color, int depth){
	//Color signifies who just took their move

	// Stalemate
	if (status == -1) {
		return (color) ? -100 : 100;
	}
	// Win
	return  (color ? 1000 : -1000) / depth;
}

int minimax(Game &game, int depth, int alpha, int beta, Move* choice) {
	MoveDetails details;

	// Get all possible moves 
	std::vector<Move> moves = game.get_all_moves(game.turn);

	// Data collection
	int i = 0;

	// Initialize games with the next move
	std::vector<std::tuple<int, Move>> sortedMoves;
	for (auto move : moves) {
		details = game.move(game.board[move.from.y][move.from.x], move.to);
		game.moveLog.push_back(details.move);

		int rating = 0;
		// Rank move from table
		if (useTransposition) {
			std::string gameString = game.toString();
			StringTable::const_accessor a;
			if (transpositionTable.find(a, gameString)) {
				rating = a->second[2];
			}
			a.release();
		}
		sortedMoves.push_back({rating, move});
		
		// Move back
		game.moveBack(details);
		game.moveLog.pop_back();
	}

	if (useTransposition) {
		// Sort them according to transposition table
		if (game.turn) {
			std::sort(sortedMoves.begin(), sortedMoves.end(), [](const std::tuple<int, Move> &a, const std::tuple<int, Move> &b) {
				return std::get<0>(a) > std::get<0>(b);
			});
		} else {
			std::sort(sortedMoves.begin(), sortedMoves.end(), [](const std::tuple<int, Move> &a, const std::tuple<int, Move> &b) {
				return std::get<0>(a) < std::get<0>(b);			
			});
		}
	}

	// Iterate through moves
	for(auto currentMove : sortedMoves) {
		i++; // Data collection
		
		int material = INT_MAX;

		Move moveToApply = std::get<1>(currentMove);
		details = game.move(game.board[moveToApply.from.y][moveToApply.from.x], moveToApply.to);
		game.moveLog.push_back(details.move);

		std::string gameString = game.toString();

		// Table lookup
		if (useTransposition) {
			StringTable::const_accessor a;
			if (transpositionTable.find(a, gameString)) {
				if (a->second[0] == depth && a->second[1] == maxdepth) {
					material = a->second[2];
				}
			}
			a.release();
		}
		
		// Evaluate node
		if(material == INT_MAX) {
			// Check for termination
			int terminated = game.check_for_winner(game.turn);
			if (terminated) {
				material = utility(terminated, game.turn, depth);
			}
			// Check for depth limit
			else if (depth == maxdepth) {
				material = heuristic(game);
			}
			// Search
			else {
				game.turn = !game.turn;
				material = minimax(game, depth + 1, alpha, beta, NULL);
				game.turn = !game.turn;
			}
			// Store result
			if (useTransposition) {
				StringTable::accessor a;
				if (transpositionTable.insert(a, gameString)) {
					a->second = {depth, maxdepth, material};
				}
				a.release();
			}
		}

		// Move back
		game.moveBack(details);
		game.moveLog.pop_back();

		int* alphaOrBeta;
		// Set best material variable for pruning
		if ((game.turn && material > *(alphaOrBeta = &alpha)) || 
			(!game.turn && material < *(alphaOrBeta = &beta))) {
			*alphaOrBeta = material;
			if (depth == 1) {
				*choice = details.move;
			}
		}

		// Prune
		if (beta <= alpha) {
			break;
		}
	}
 	
	// Data collection
	visited[depth] += i;

	return game.turn ? alpha : beta;
}

Move call_minimax(Game &game) {
	Move choice;
	int ret = minimax(game, 1, INT_MIN, INT_MAX, &choice);
	// printf("Minimax result: %d Move: %s\n", ret, choice.toString().c_str());
	// Data collection
	printStats();

	return choice;
}

void thread_func_minimax(int, Game &game, Move move, std::mutex* writeLock, int* alpha, int* beta, Move* choice) {
	visited[1]++;

	Game copy = game;
	// Try move - pointer to piece no longer valide, must use positions
	MoveDetails details = copy.move(copy.board[move.from.y][move.from.x], move.to);
	copy.moveLog.push_back(details.move);

	int material;
	// Check for termination
	int terminated = copy.check_for_winner(copy.turn);
	if (terminated) {
		material = utility(terminated, copy.turn, 1);
	}
	// Check for depth limit
	else if (maxdepth == 1) {
		material = heuristic(copy);
	} else {
		// Get updated cutoff to reflect current search progress
		writeLock->lock();
		int a = *alpha;
		int b = *beta;
		writeLock->unlock();
		// Search
		copy.turn = !copy.turn;
		material = minimax(copy, 2, a, b, NULL);
	}
	if (useTransposition) {
		StringTable::accessor a;
		if (transpositionTable.insert(a, game.toString())) {
			a->second = {1, maxdepth, material};
		}
		a.release();
	}

	// Update choice if we have a better move
	writeLock->lock();
	int* alphaOrBeta;
	// Set best material variable for pruning
	if ((game.turn && material > *(alphaOrBeta = alpha)) || 
		(!game.turn && material < *(alphaOrBeta = beta))) {
		*alphaOrBeta = material;
		*choice = details.move;
	}
	writeLock->unlock();
}

Move call_minimax_fast(Game& game) {
	std::vector<Move> moves = game.get_all_moves(game.turn);
	int alpha = INT_MIN;
	int beta = INT_MAX;
	Move choice;

	std::mutex writeLock;
	ctpl::thread_pool p(16); /* sixteen threads in the pool */
	std::vector<std::future<void>> results(moves.size());

	// TODO: Order moves
	for (unsigned int i = 0; i < moves.size(); ++i){
		results[i] = p.push(thread_func_minimax, game, moves[i], &writeLock, &alpha, &beta, &choice);
	}

	for (unsigned int i = 0; i < moves.size(); ++i) {
		results[i].get();
	}

	// Data collection
	printStats();
	// printf("Depth %d: %d Move: %s\n", maxdepth, game.turn ? alpha : beta, choice.toString().c_str());

	return choice;
}

Move call_minimax_IDS(Game &game) {
	//Enable transposition and empty the transpostion table
	useTransposition = true;
	clearTable();

	Move move;
	for (int i = 1; i <= MAXDEPTH; i += 1) {
		maxdepth = i;
		move = call_minimax(game);
	}

	// Disable transposition and empty the transpostion table
	clearTable();
	useTransposition = false;

	return move;
}

Move call_minimax_IDS_fast(Game &game) {
	//Enable transposition and empty the transpostion table
	useTransposition = true;
	clearTable();

	Move move;
	for (int i = 1; i <= MAXDEPTH; i += 1) {
		maxdepth = i;
		move = call_minimax_fast(game);
	}
	maxdepth = MAXDEPTH;
	
	// Disable transposition and empty the transpostion table
	clearTable();
	useTransposition = false;

	return move;
}

Move move_with_opening(Game& game, Move (*func)(Game&)) {
	std::string bookMove;
	std::string moveHistory = "";
	// Build move history string
	for (auto i : game.moveLog) {
		moveHistory += i.toString()+" ";
	}
	// Search for a response in the opening database
	if (game.moveLog.size() < MAXLENGTH &&
		lookupMove(moveHistory, bookMove)) {
			return Move().translate(bookMove);
	}
	// If the game is out of book, call the continuation function
	return func(game);
}

void printStats() {
    printf("{");
    for (int i = 1; i <= MAXDEPTH; i++) {
        printf("%d  ", visited[i]);
    }
    printf("}\n");
    int visitedN[MAXDEPTH+1] = {0};
    memcpy(visited, visitedN, sizeof(int) * (MAXDEPTH+1));
}

void clearTable(){
	transpositionTable.clear();
}
