#include "Engine.h"
#include <thread>
#include <mutex>
#include <iostream>
#include "ctpl_stl.h"
#include <climits>

int heuristic(Game &game) {
	int material = 0;
	// Lookup table
	int worth[] = { 1,3,3,5,9,0 };

	// White pieces
	for (auto piece : game.whitePieces) {
		if (piece != NULL) {
			material += worth[piece->piece];
		}
	}
	// Black pieces
	for (auto piece : game.blackPieces) {
		if (piece != NULL) {
			material -= worth[piece->piece];
		}
	}
	return material;
}

int utility(int status, bool color, int depth){
	// Stalemate
	if (status == -1) {
		return (color) ? -100 : 100;
	}
	// Win
	return  (color ? 1000 : -1000) / depth;
}
int pruned[10] = {0,0,0,0,0,0,0,0,0,0};
int visited[10] = {0,0,0,0,0,0,0,0,0,0};
int minimax(Game &game, int depth, bool color, int bestChoice) {
	std::vector<move_info> moves = game.get_all_moves(color);
	int bestMat = color ? INT_MIN : INT_MAX;

	int total = moves.size();// Data collection
	int i = 0;// Data collection
	visited[depth] += total; // Data collection
	// Iterate through possible moves
	for (auto move : moves) {
		i++; // Data collection
		// Make a move
		move_info log = game.try_move(move.piece, move.to);
		game.move_log.push_back(log);

		int material = 0;
		// Check for termination
		int terminated = game.check_for_winner(color);
		if (terminated) {
			material = utility(terminated, color, depth);
		}
		// Check for depth limit
		else if (depth == 5) {
			material = heuristic(game);
		}
		// Recurse
		else {
			material = minimax(game, depth + 1, !color, bestMat);
		}

		// Undo move
		game.move_back(log);
		game.move_log.pop_back();

		// Set best material variable for pruning
		if ((color && material > bestMat) || (!color && material < bestMat)) {
			bestMat = material;
		}

		// Prune
		if ((color && material >= bestChoice) || (!color && material <= bestChoice)) {
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
		log = game.try_move(move.piece, move.to);
		game.move_log.push_back(log);

		// Call minimax
		int material = minimax(game, 2, !game.turn, bestMat);

		// Undo move
		game.move_back(log);
		game.move_log.pop_back();

		// Keep best move for decision
		if ((game.turn && material > bestMat) || (!game.turn && material < bestMat)) {
			bestMat = material;
			choice = move;
		}
	}
	// Take the chosen move
	game.log_move(choice.piece, choice.to);

	// Data collection
	printf("{");
	for (int i = 0; i < 10; i++) {
		printf("%d/%d  ", pruned[i], visited[i]);
	}
	printf("}\n");
}

void run_minimax(int, Game game, move_info move, std::mutex* writeLock, int* bestMat, move_info* choice) {
	visited[1]++;
	// Try move - pointer to piece no longer valide, must use positions
	move_info log = game.try_move(game.board[move.from.y][move.from.x], move.to);
	game.move_log.push_back(log);

	int material;
	// Check for termination
	int terminated = game.check_for_winner(game.turn);
	if (terminated) {
		material = utility(terminated, game.turn, 1);
	}
	else {
		// Update cutoff to reflect current search progress
		writeLock->lock();
		int cutOff = *bestMat;
		writeLock->unlock();
		// Recurse
		material = minimax(game, 2, !game.turn, cutOff);
	}

	// Update choice if we have a better move
	writeLock->lock();
	if ((game.turn && material > *bestMat) || (!game.turn && material < *bestMat)) {
		*bestMat = material;
		*choice = log;
	}
	writeLock->unlock();
	// std::cout << material << std::endl;

	// Undo move
	game.move_back(log);
	game.move_log.pop_back();
}


void take_move_fast(Game& game) {
	// DATA COLLECTION
	

	std::vector<move_info> moves = game.get_all_moves(game.turn);
	int bestMat = game.turn ? INT_MIN : INT_MAX;
	move_info choice = move_info{ {-1, -1}, {-1, -1}, NULL, NULL, false };

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
	// printf("{%d/%d,%d/%d,%d/%d,%d/%d,%d/%d,%d/%d,%d/%d,%d/%d,%d/%d,%d/%d}\n", pruned[0],visited[0],pruned[1],visited[1],pruned[2],visited[2],pruned[3],visited[3],pruned[4],visited[4],pruned[5],visited[5],pruned[6],visited[6],pruned[7],visited[7],pruned[8],visited[8],pruned[9],visited[9]);
	game.log_move(game.board[choice.from.y][choice.from.x], choice.to);
}