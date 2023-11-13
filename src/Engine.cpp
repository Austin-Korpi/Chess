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

#define MAXDEPTH 5
int maxdepth = MAXDEPTH;

// Global Data collection variables
int visited[MAXDEPTH+1] = {0};

std::unordered_map<std::string, std::array<int, 3>> transpositionTable;
std::mutex tableLock;
bool useTransposition = false;

int heuristic(Game &game) {
	int material = 0;
	// Lookup table
	int worth[] = { 1,3,3,5,9,0 };

	// White pieces
	for (auto piece : game.whitePieces) {
		if (!piece.captured) {
			material += worth[piece.type];
		}
	}
	// Black pieces
	for (auto piece : game.blackPieces) {
		if (!piece.captured) {
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

int minimax(Game &game, int depth, int alpha, int beta, move_info* choice) {
	move_info log;

	// Get all possible moves 
	std::vector<move_info> moves = game.get_all_moves(game.turn);

	// Data collection
	int i = 0;

	// Initialize games with the next move
	std::vector<std::tuple<int, Game*>> sortedGames;
	for (auto move : moves) {
		Game* copy = new Game(game);
		log = copy->move(copy->board[move.from.y][move.from.x], move.to);
		copy->moveLog.push_back(log);

		int rating = 0;
		// Rank move from table
		if (useTransposition) {
			std::string gameString = copy->toString();
			tableLock.lock();
			if (transpositionTable.count(gameString)) {
				rating = transpositionTable[gameString][2];
			}
			tableLock.unlock();
		}
		sortedGames.push_back({rating, copy});
	}

	if (useTransposition) {
		// Sort them according to transposition table
		if (game.turn) {
			std::sort(sortedGames.begin(), sortedGames.end(), [](const std::tuple<int, Game*> &a, const std::tuple<int, Game*> &b) {
				return std::get<0>(a) > std::get<0>(b);
			});
		} else {
			std::sort(sortedGames.begin(), sortedGames.end(), [](const std::tuple<int, Game*> &a, const std::tuple<int, Game*> &b) {
				return std::get<0>(a) < std::get<0>(b);			
			});
		}
	}

	// Iterate through game
	for(auto currentGame : sortedGames) {
		i++; // Data collection
		
		int material = INT_MAX;

		Game copy = *std::get<1>(currentGame);
		std::string gameString = copy.toString();

		// Table lookup
		if (useTransposition) {
			std::array<int, 3> entry;
			tableLock.lock();
			if (transpositionTable.count(gameString) && ((entry = transpositionTable[gameString])[0] == depth)
			&& entry[1] == maxdepth) {
				material = entry[2];
			}
			tableLock.unlock();
		}
		
		// Evaluate node
		if(material == INT_MAX) {
			// Check for termination
			int terminated = copy.check_for_winner(copy.turn);
			if (terminated) {
				material = utility(terminated, copy.turn, depth);
			}
			// Check for depth limit
			else if (depth == maxdepth) {
				material = heuristic(copy);
			}
			// Search
			else {
				copy.turn = !copy.turn;
				material = minimax(copy, depth + 1, alpha, beta, NULL);
			}
			// Store result
			if (useTransposition) {
				tableLock.lock();
				transpositionTable[gameString] = {depth, maxdepth, material};
				tableLock.unlock();
			}
		}

		int* alphaOrBeta;
		// Set best material variable for pruning
		if ((game.turn && material > *(alphaOrBeta = &alpha)) || 
			(!game.turn && material < *(alphaOrBeta = &beta))) {
			*alphaOrBeta = material;
			if (depth == 1) {
				*choice = copy.moveLog.back();
			}
		}

		// Prune
		if (beta <= alpha) {
			break;
		}
	}

	// Free memory
	for (unsigned long i = 0; i < sortedGames.size(); i++) {
		delete std::get<1>(sortedGames[i]);
	}
 	
	// Data collection
	visited[depth] += i;

	return game.turn ? alpha : beta;
}

move_info call_minimax(Game &game) {
	move_info choice;
	minimax(game, 1, INT_MIN, INT_MAX, &choice);

	// Data collection
	printf("{");
	for (int i = 1; i <= MAXDEPTH; i++) {
		printf("%d  ", visited[i]);
	}
	printf("}\n");
	int visitedN[MAXDEPTH+1] = {0};
	memcpy(visited, visitedN, sizeof(int) * (MAXDEPTH+1));

	return choice;
}

void thread_func_minimax(int, Game &game, move_info move, std::mutex* writeLock, int* alpha, int* beta, move_info* choice) {
	visited[1]++;

	Game copy = game;
	// Try move - pointer to piece no longer valide, must use positions
	move_info log = copy.move(copy.board[move.from.y][move.from.x], move.to);
	copy.moveLog.push_back(log);

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
		transpositionTable[game.toString()] = {1, maxdepth, material};
	}

	// Update choice if we have a better move
	writeLock->lock();
	int* alphaOrBeta;
	// Set best material variable for pruning
	if ((game.turn && material > *(alphaOrBeta = alpha)) || 
		(!game.turn && material < *(alphaOrBeta = beta))) {
		*alphaOrBeta = material;
		*choice = log;
	}
	writeLock->unlock();
}

move_info call_minimax_fast(Game& game) {
	std::vector<move_info> moves = game.get_all_moves(game.turn);
	int alpha = INT_MIN;
	int beta = INT_MAX;
	move_info choice;

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
	printf("{");
	for (int i = 1; i < MAXDEPTH + 1; i++) {
		printf("%d  ", visited[i]);
	}
	printf("}\n");
	int visitedN[MAXDEPTH+1] = {0};
	memcpy(visited, visitedN, sizeof(int) * (MAXDEPTH+1));

	return choice;
}

move_info call_minimax_IDS(Game &game) {
	//Enable transposition and empty the transpostion table
	useTransposition = true;
	transpositionTable = std::unordered_map<std::string, std::array<int, 3>>();

	move_info move;
	for (int i = 1; i <= MAXDEPTH; i += 1) {
		maxdepth = i;
		move = call_minimax(game);
	}

	// Disable transposition and empty the transpostion table
	transpositionTable = std::unordered_map<std::string, std::array<int, 3>>();
	useTransposition = false;

	return move;
}

move_info call_minimax_IDS_fast(Game &game) {
	//Enable transposition and empty the transpostion table
	useTransposition = true;
	transpositionTable = std::unordered_map<std::string, std::array<int, 3>>();

	move_info move;
	for (int i = 1; i <= MAXDEPTH-1; i += 1) {
		maxdepth = i;
		move = call_minimax_fast(game);
	}
	maxdepth = MAXDEPTH;
	call_minimax_fast(game);
	
	// Disable transposition and empty the transpostion table
	transpositionTable = std::unordered_map<std::string, std::array<int, 3>>();
	useTransposition = false;

	return move;
}

move_info move_with_opening(Game& game, move_info (*func)(Game&)) {
	std::string bookMove;
	std::string moveHistory = "";
	// Build move history string
	for (auto i : game.moveLog) {
		moveHistory += i.toString()+" ";
	}
	// Search for a response in the opening database
	if (game.moveLog.size() < MAXLENGTH &&
		lookupMove(moveHistory, bookMove)) {
			return move_info().translate(bookMove);
	}
	// If the game is out of book, call the continuation function
	return func(game);
}