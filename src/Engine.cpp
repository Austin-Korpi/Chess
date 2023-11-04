#include "Engine.h"
#include <thread>
#include <mutex>
#include <iostream>
#include "ctpl_stl.h"
#include <climits>

#define MAXDEPTH 7

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

// Global Data collection variables
int pruned[10] = {0,0,0,0,0,0,0,0,0,0};
int visited[10] = {0,0,0,0,0,0,0,0,0,0};
int minimax(Game &game, int depth, int bestChoice) {
	std::vector<move_info> moves = game.get_all_moves(game.turn);
	int bestMat = game.turn ? INT_MIN : INT_MAX;

	int total = moves.size();// Data collection
	int i = 0;// Data collection
	visited[depth] += total; // Data collection
	// Iterate through possible moves
	for (auto move : moves) {
		i++; // Data collection
		// Make a move
		Game copy = game;
		move_info log = copy.try_move(copy.board[move.from.y][move.from.x], move.to);
		copy.moveLog.push_back(log);

		int material = 0;
		// Check for termination
		int terminated = copy.check_for_winner(copy.turn);
		if (terminated) {
			material = utility(terminated, copy.turn, depth);
		}
		// Check for depth limit
		else if (depth == MAXDEPTH) {
			material = heuristic(copy);
		}
		// Recurse
		else {
			copy.turn = !copy.turn;
			material = minimax(copy, depth + 1, bestMat);
		}

		// Set best material variable for pruning
		if ((game.turn && material > bestMat) || (!game.turn && material < bestMat)) {
			bestMat = material;
		}

		// Prune
		if ((game.turn && material >= bestChoice) || (!game.turn && material <= bestChoice)) {
			pruned[depth] += total - i;// Data collection
			break;
		}
	}
	return bestMat;
}

void take_move(Game &game) {
	move_info log;
	move_info choice;
	int bestMat = game.turn ? INT_MIN : INT_MAX;

	// Get all possible moves 
	// There should be at least one, or the game would have been over
	std::vector<move_info> moves = game.get_all_moves(game.turn);
	
	visited[1] += moves.size();
	for (auto move : moves) {
		// Take move
		Game copy = game;
		log = copy.try_move(copy.board[move.from.y][move.from.x], move.to);
		copy.moveLog.push_back(log);
		copy.turn = !copy.turn;

		// Call minimax
		int material = minimax(copy, 2, bestMat);

		// Keep best move for decision
		if ((game.turn && material > bestMat) || (!game.turn && material < bestMat)) {
			bestMat = material;
			choice = move;
		}
	}
	// Take the chosen move
	game.log_move(choice);

	// Data collection
	printf("{");
	for (int i = 0; i < 10; i++) {
		printf("%d/%d  ", pruned[i], visited[i]);
	}
	printf("}\n");
}

void run_minimax(int, Game &game, move_info move, std::mutex* writeLock, int* bestMat, move_info* choice) {
	visited[1]++;

	Game copy = game;
	// Try move - pointer to piece no longer valide, must use positions
	move_info log = copy.try_move(copy.board[move.from.y][move.from.x], move.to);
	copy.moveLog.push_back(log);

	int material;
	// Check for termination
	int terminated = copy.check_for_winner(copy.turn);
	if (terminated) {
		material = utility(terminated, copy.turn, 1);
	}
	else {
		// Update cutoff to reflect current search progress
		writeLock->lock();
		int cutOff = *bestMat;
		writeLock->unlock();
		// Recurse
		copy.turn = !copy.turn;
		material = minimax(copy, 2, cutOff);
	}

	// Update choice if we have a better move
	writeLock->lock();
	if ((game.turn && material > *bestMat) || (!game.turn && material < *bestMat)) {
		*bestMat = material;
		*choice = log;
	}
	writeLock->unlock();
	// std::cout << material << std::endl;
}


void take_move_fast(Game& game) {
	// DATA COLLECTION

	std::vector<move_info> moves = game.get_all_moves(game.turn);
	int bestMat = game.turn ? INT_MIN : INT_MAX;
	move_info choice = move_info{ {-1, -1}, {-1, -1}};//, NULL, NULL, false };

	std::mutex writeLock;
	ctpl::thread_pool p(16); /* sixteen threads in the pool */
	std::vector<std::future<void>> results(moves.size());

	for (unsigned int i = 0; i < moves.size(); ++i){
		results[i] = p.push(run_minimax, game, moves[i], &writeLock, &bestMat, &choice);
	}

	for (unsigned int i = 0; i < moves.size(); ++i) {
		results[i].get();
	}

	// std::cout << "Max: " << bestMat << std::endl;
	// std::cout << "[" << choice.from.x << ", " << choice.from.y << "] [" << choice.to.x << ", " << choice.to.y << "]" << choice.piece->piece << std::endl;
	printf("{");
	for (int i = 0; i < 10; i++) {
		printf("%d/%d  ", pruned[i], visited[i]);
	}
	printf("}\n");
	game.log_move(choice);
}