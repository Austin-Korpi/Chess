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

#define R 3
int maxdepth = MAXDEPTH;
bool null_ok = false;

// Global Data collection variables
int visited[21] = {0};

typedef oneapi::tbb::concurrent_hash_map<std::string, std::array<int, 3>> StringTable;
StringTable transpositionTable;
std::mutex tableLock;
bool useTransposition = false;

int heuristic(Game &game) {
	int material = 0;
	// Lookup table
	int worth[] = { 100,320,330,500,900,0 };

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
	return game.turn ? material : -material;
}

int utility(int status, bool color, int depth){
	//Color signifies who just took their move

	// Stalemate
	if (status == -1) {
		return -20000;
	}
	// Win
	return  400000 / depth;
}

void sortMovesCaptured(std::vector<Move>& moves, Game &game) {
	bool sortNeeded = false;
	std::vector<int> scores(moves.size());
	for (unsigned int i = 0; i < moves.size(); i++) {
		Piece* piece = game.board[moves[i].from.y][moves[i].from.x];
		if (piece) {
			sortNeeded = true;
			scores[i] = piece->type;
		} else {
			scores[i] = -1;
		}
	}
	if (sortNeeded) {
		int j, key;
		for (unsigned int i = 1; i < scores.size(); i++) {
			key = scores[i];
			Move value = moves[i];
			j = i - 1;

			while (j >= 0 && scores[j] < key) {
				scores[j + 1] = scores[j];
				moves[j+1] = moves[j];
				j = j - 1;
			}
			scores[j + 1] = key;
			moves[j+1] = value;
		}
	}
}

void sortMovesTable(std::vector<Move>& moves, Game &game) {
	bool sortNeeded = false;
	// Initialize scores
	std::vector<int> scores;
	for (auto move : moves) {
		MoveDetails details = game.move(game.board[move.from.y][move.from.x], move.to);
		game.moveLog.push_back(details.move);
		int rating = 0;
		std::string gameString = game.toString();
		StringTable::const_accessor a;
		if (transpositionTable.find(a, gameString)) {
			rating = a->second[2];
			sortNeeded = true;
		}
		a.release();
		scores.push_back(rating);
		game.moveBack(details);
		game.moveLog.pop_back();
	}
	if (sortNeeded) {
		int j, key;
		for (unsigned int i = 1; i < scores.size(); i++) {
			key = scores[i];
			Move value = moves[i];
			j = i - 1;
	
			while (j >= 0 && scores[j] < key) {
				scores[j + 1] = scores[j];
				moves[j+1] = moves[j];
				j = j - 1;
			}
			scores[j + 1] = key;
			moves[j+1] = value;
		}
	}
}

int quiesce(Game& game, int alpha, int beta, int depth) {
	// Get default value
    int stand_pat = heuristic(game);
	// Check beta
    if( stand_pat >= beta )
        return beta;
	// Check alpha
    if( alpha < stand_pat ) {
        alpha = stand_pat;
	}
	// Get all moves
	std::vector<Move> moves = game.get_all_moves(game.turn);
	 
	// Sort moves by value of piece being captured
	sortMovesCaptured(moves, game);

	// Loop through moves
    for(auto move : moves)  {
		if (game.board[move.to.y][move.to.x]) {
			//Move
			MoveDetails details = game.move(game.board[move.from.y][move.from.x], move.to);
			game.moveLog.push_back(details.move);   
			
			// visited[depth]++; // Data collection

			// Continue the search
			game.turn = !game.turn;
			int score = -quiesce(game, -beta, -alpha, depth+1);
			game.turn = !game.turn;
			
			// Move back
			game.moveBack(details);
			game.moveLog.pop_back();

			// Check beta
			if( score >= beta )
				return beta;

			// Update alpha
			if( score > alpha )
				alpha = score;
		} 
		// Don't search non-capture moves
		else {
			break;
		}
    }
	// alpha is the highest material found
    return alpha;
}

int minimax(Game &game, int depth, int alpha, int beta, Move* choice, bool verify) {
	MoveDetails details;

	// Get all possible moves 
	std::vector<Move> moves = game.get_all_moves(game.turn);
	
	// Sort for better cutoffs
	sortMovesCaptured(moves, game);
	if (useTransposition) {
		sortMovesTable(moves, game);
	}

	int bestMat = INT_MIN;
	// Iterate through moves
	for(auto currentMove : moves) {
		visited[depth]++; // Data collection
		
		int material = INT_MAX;

		details = game.move(game.board[currentMove.from.y][currentMove.from.x], currentMove.to);
		game.moveLog.push_back(details.move);

		// Table lookup
		std::string gameString = game.toString();
		if (useTransposition) {
			StringTable::const_accessor a;
			if (transpositionTable.find(a, gameString)) {
				if (a->second[0] == depth && a->second[1] == maxdepth) {
					material = a->second[2];
				}
			}
		} 
		
		bool fail_high = false;
		// Evaluate node
		if(material == INT_MAX) {
			// Check for termination
			int terminated = game.check_for_winner(game.turn);
			if (terminated) {
				material = utility(terminated, game.turn, depth);
			}
			// Check for depth limit
			else if (depth >= maxdepth) {
				// material = heuristic(game);
				material = quiesce(game, alpha, beta, depth+1);
				// printf("quiesce: %d\n", material);
			}
			// Search
			else {
				// Null Search
				if (!game.check_for_check(game.turn) && null_ok && (!verify || depth < maxdepth-1)) {
					/* null-move search with minimal window around beta */
					// int value = -minimax(game, depth+R+1,-beta, -beta+1, NULL, verify);
					int value = minimax(game, depth+R+1, beta-1, beta, NULL, verify);
					if (value >= beta) {

						/* fail-high */
						if (verify) {
							/* reduce the depth by one ply */
							depth++;
							// printf("-");
							/* turn verification off for the sub-tree */
							verify = false;
							/* mark a fail-high flag, to detect zugzwangs later*/
							fail_high = true;
						}
						else {
							/* cutoff in a sub-tree with fail-high report */
							// Move back
							game.moveBack(details);
							game.moveLog.pop_back();
							// for (int i = 0; i < depth; i++) {
							// 	printf("  ");
							// }
							// printf("Depth %d. S=%d, a=%d, b=%d\n", depth, material, alpha, beta);
							// printf("prune");
							return value;
							// bestMat = value;
						}
					}
				}
			research:
				// Search
				game.turn = !game.turn;
				material = -minimax(game, depth + 1, -beta, -alpha, NULL, verify);
				game.turn = !game.turn;
				
				// Verify
				if(fail_high && material < beta) {
					depth--;
					fail_high = false;
					verify = true;
					// printf("|");
					goto research;
				}
			}
			// Store result
			if (useTransposition) {
				StringTable::accessor a;
				if (transpositionTable.insert(a, gameString)) {
					a->second = {depth, maxdepth, material};
				}
			}
		}

		// Move back
		game.moveBack(details);
		game.moveLog.pop_back();

		// for (int i = 0; i < depth; i++) {
		// 	printf("  ");
		// }
		// printf("Depth %d. S=%d, a=%d, b=%d\n", depth, material, alpha, beta);
		
		// Set best material variable for pruning
		if (material >= beta) {
			return material;
		}
		if (material > bestMat) {
			bestMat = material;
			if (depth == 1) {
				*choice = details.move;
			}
			if (material > alpha) {
				alpha = material;
			}
		}
	}

	return bestMat;
}

Move call_minimax(Game &game) {
	Move choice;
	minimax(game, 1, -INT_MAX, INT_MAX, &choice, true);
	// printf("Depth 0: %d\n", ret);
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
		material = -minimax(copy, 2, -b, -a, NULL, true);
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
	
	// Set best material variable for pruning	
	if (material > *alpha) {
		*alpha = material;
		*choice = details.move;
	}	
	writeLock->unlock();
}

Move call_minimax_fast(Game& game) {
	std::vector<Move> moves = game.get_all_moves(game.turn);
	int alpha = -INT_MAX;
	int beta = INT_MAX;
	Move choice;

	sortMovesCaptured(moves, game);
	if (useTransposition) {
		sortMovesTable(moves, game);
	}

	std::mutex writeLock;
	ctpl::thread_pool p(16); /* sixteen threads in the pool */
	std::vector<std::future<void>> results(moves.size());

	// TODO: Order moves
	for (unsigned int i = 0; i < moves.size(); ++i){
		// results[i] = p.push(thread_func_minimax, game, moves[i], &writeLock, &alpha, &beta, &choice);
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
	// null_ok = true;
	clearTable();

	Move move;
	for (int i = 1; i <= MAXDEPTH; i += 1) {
		maxdepth = i;
		move = call_minimax(game);
	}

	// Disable transposition and empty the transpostion table
	clearTable();
	useTransposition = false;
	null_ok = false;

	return move;
}

Move call_minimax_IDS_fast(Game &game) {
	//Enable transposition and empty the transpostion table
	useTransposition = true;
	// null_ok = true;
	clearTable();

    auto start = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(start - start);

	Move move;
	maxdepth = 0;
	for (int i = 1; i <= MAXDEPTH; i += 1) {
    // while (duration.count() < 1000) {
		maxdepth ++;
		move = call_minimax_fast(game);
		auto end = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	}
    printf("IDS maxdepth: %d\n", maxdepth);
	maxdepth = MAXDEPTH;

    // printf("IDS: %s\n", move.toString().c_str());
	
	// Disable transposition and empty the transpostion table
	clearTable();
	null_ok = false;
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
    for (int i = 1; i <= 10; i++) {
        printf("%d  ", visited[i]);
    }
    printf("}\n");
    int visitedN[11] = {0};
    memcpy(visited, visitedN, sizeof(int) * 11);
}

void clearTable(){
	transpositionTable.clear();
}
