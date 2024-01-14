#include <thread>
#include <mutex>
#include <iostream>
#include <climits>
#include <cstring>
#include <unordered_map>
#include <algorithm>
#include "ctpl_stl.h"
#include <sstream>

#include "Engine.h"
#include "Opening.h"
#include "oneapi/tbb/concurrent_hash_map.h"

#define TIMER true
#define R 3

int maxdepth = MAXDEPTH;
bool null_ok = false;
bool useLMR = false;
bool useQuiesce = true;
bool useTransposition = false;
bool extendEval = true;

// Global Data collection variables
int nodeCount = 0;

// Transposition Table
typedef oneapi::tbb::concurrent_hash_map<std::string, std::array<int, 3>> StringTable;
StringTable transpositionTable;
std::mutex tableLock;

// Timer
volatile bool timeUp = false;
void waitForTimeAndChangeVariable(int seconds) {
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
    timeUp = true;
}

// Midstate evalutation function
int heuristic(Game &game) {
	int material = 0;
	// Lookup table
	int worth[] = { 100,320,330,500,900,0 };

	// White Material
	for (auto piece : game.whitePieces) {
		if (piece.white >= 0) {
			material += worth[piece.type];
		}
	}
	// Black Material
	for (auto piece : game.blackPieces) {
		if (piece.white >= 0) {
			material -= worth[piece.type];
		}
	}

	if (!extendEval) {
		return material;
	}

	// Pawn penalties
	material -= (game.count_bid_pawns(true) - game.count_bid_pawns(false)) * 10;

	// Passed Pawn
	material += (game.count_passed_pawns(true) - game.count_passed_pawns(false)) * 10;

	// Mobility
	// material += (game.get_all_moves(game.turn).size() - game.get_all_moves(!game.turn).size()) * 4;

	// Bishop Pair
	if (game.whitePieces[10].white > -1 && game.whitePieces[11].white > -1)
		material += 30;
	if (game.blackPieces[10].white > -1 && game.blackPieces[11].white > -1)
		material -= 30;

	// Rook Attack
	material += (game.count_rook_attacks(true) - game.count_rook_attacks(false)) * 5;

	// Castling
	if (game.hasCastled & 0b10)
		material += 20;
	if (game.hasCastled & 0b01)
		material -= 20;	

	return game.turn ? material : -material;
}

int utility(int status, bool color, int depth){
	//Color signifies who just took their move

	// Stalemate
	if (status == -1) {
		return -20000;
	}
	// Win
	return  400000 - depth;
}

void sortMovesCaptured(std::vector<Move>& moves, Game &game) {
	bool sortNeeded = false;
	std::vector<int> scores(moves.size());
	for (unsigned int i = 0; i < moves.size(); i++) {
		Piece* piece = game.board[moves[i].to.y][moves[i].to.x];
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
	int sortNeeded = 0;
	// Initialize scores
	std::vector<int> scores;
	for (auto move : moves) {
		MoveDetails details = game.move(game.board[move.from.y][move.from.x], move.to);
		game.moveLog.push_back(details.move);
		int rating = INT_MIN;
		std::string gameString = game.toString();
		StringTable::const_accessor a;
		if (transpositionTable.find(a, gameString)) {
			rating = a->second[2];
			sortNeeded++;
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
			
			nodeCount++;

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
	if (timeUp) {
		return alpha;
	}

	MoveDetails details;

	// Get all possible moves 
	std::vector<Move> moves = game.get_all_moves(game.turn);
	
	// Sort for better cutoffs
	sortMovesCaptured(moves, game);
	if (useTransposition && maxdepth - depth > 1) {
		sortMovesTable(moves, game);
	}

	int bestMat = INT_MIN;
	// Iterate through moves
	for(auto currentMove : moves) {
		nodeCount++;
		
		int material = INT_MAX;
		int i = 0;

		details = game.move(game.board[currentMove.from.y][currentMove.from.x], currentMove.to);
		game.moveLog.push_back(details.move);

		// Table lookup
		std::string gameString;
		if (useTransposition) {
			gameString = game.toString();
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
				if (useQuiesce) {
					game.turn = !game.turn;
					material = -quiesce(game, -beta, -alpha, depth+1);
					game.turn = !game.turn;	
				} else {
					material = heuristic(game); 
				}
			}
			// Search
			else {
				// Null Search
				if (null_ok && !game.check_for_check(game.turn) && (!verify || depth < maxdepth-1)) {
					/* null-move search with minimal window around beta */
					// int value = -minimax(game, depth+R+1,-beta, -beta+1, NULL, verify);
					int value = minimax(game, depth+R+1, beta-1, beta, NULL, verify);
					if (value >= beta) {

						/* fail-high */
						if (verify) {
							/* reduce the depth by one ply */
							depth++;
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

							return value;
						}
					}
				}
			research:
				// Search
				game.turn = !game.turn;
				if (!useLMR || i == 0 || maxdepth - depth < 3 || details.captured || game.check_for_check(game.turn)) {
					// Search 
					material = -minimax(game, depth + 1, -beta, -alpha, NULL, verify);
				} else {
					//Late Move Reduction
					material = -minimax(game, depth + 2, -beta, -alpha, NULL, verify);
					if (material > alpha) {
						material = -minimax(game, depth + 1, -beta, -alpha, NULL, verify);
					}
				}				
				game.turn = !game.turn;
				
				// Verify
				if(fail_high && material < beta) {
					depth--;
					fail_high = false;
					verify = true;
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
		i++;
	}

	return bestMat;
}

Move call_minimax(Game &game) {
	Move choice;
	minimax(game, 1, -INT_MAX, INT_MAX, &choice, true);
	// Data collection
	// printStats();
	printf("%d, ", nodeCount);
	nodeCount = 0;
	return choice;
}

void thread_func_minimax(int, Game &game, Move move, std::mutex* writeLock, int* alpha, int* beta, Move* choice) {
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

	for (unsigned int i = 0; i < moves.size(); ++i){
		results[i] = p.push(thread_func_minimax, game, moves[i], &writeLock, &alpha, &beta, &choice);
	}

	for (unsigned int i = 0; i < moves.size(); ++i) {
		results[i].get();
	}

	// Data collection
	// printf("Depth %d: %d Move: %s\n", maxdepth, game.turn ? alpha : beta, choice.toString().c_str());

	return choice;
}

Move call_minimax_IDS(Game &game) {
	//Enable transposition and empty the transpostion table
	useTransposition = true;
	clearTable();

	Move move;
	if (TIMER) {
		timeUp = false;
		std::thread timerThread(waitForTimeAndChangeVariable, MAX_RUN_TIME);
		maxdepth = 1;
		while (!timeUp) {
			minimax(game, 1, -INT_MAX, INT_MAX, &move, true);
			maxdepth++;
		}
		timerThread.join();
		timeUp = false;
	} else {	
		maxdepth = 1;
		for (int i = 0; i < MAXDEPTH; i++) {
			minimax(game, 1, -INT_MAX, INT_MAX, &move, true);
			maxdepth++;
		}
		maxdepth = MAXDEPTH;
	}

	// printf("%d, ", nodeCount);
	// nodeCount = 0;

	// Disable transposition and empty the transpostion table
	clearTable();
	useTransposition = false;

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

void clearTable(){
	transpositionTable.clear();
}

int evaluate_board(Game& game){
	return quiesce(game, -INT_MAX, INT_MAX, 1);
}

int main() {
	std::string line;
	while (std::getline(std::cin, line)){
		// fprintf(stderr, "read:  %s\n", line.c_str());
		Game state;

		std::istringstream iss(line);

		// Temporary variable to store each word
		std::string word;

		// Read words using a loop
		while (iss >> word) {
			// Process or store each word as needed
			Move inputMove;
			inputMove.translate(word);
			state.log_move(inputMove);
		}

		Move choice = move_with_opening(state, call_minimax_IDS);
		std::cout << choice.toString() << std::endl;
	}
}