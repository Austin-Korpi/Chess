#include "Engine.h"
#include <thread>
#include <mutex>
#include <iostream>
#include "ctpl_stl.h"


int calculate_material(Game &game) {
	int material = 0;
	int worth[] = { 1,3,3,5,9,0 };

	for (auto piece : game.whitePieces) {
		if (piece != NULL) {
			material += worth[piece->piece];
		}
	}

	for (auto piece : game.blackPieces) {
		if (piece != NULL) {
			material -= worth[piece->piece];
		}
	}
	return material;
}

int minimax(Game &game, int depth, bool color, int bestChoice) {
	std::vector<move_info> moves = game.get_all_moves(color);
	int bestMat = color ? -2000 : 2000;

	
	for (auto move : moves) {
		move_info log = game.try_move(move.piece, move.to);
		game.move_log.push_back(log);

		int material = 0;
		std::string winner = game.check_for_winner(color);
		if (winner != "") {
			if (winner == "Stalemate") {
				material = color ? -100 : 100;
			}
			else {
				material = (color ? 1000 : -1000) / depth;
			}
		}
		else if (depth == 7) {
			material = calculate_material(game);
		}
		else {
			material = minimax(game, depth + 1, !color, bestMat);
		}
		if ((color && material > bestMat) || (!color && material < bestMat)) {
			bestMat = material;
		}

		game.move_back(log);
		game.move_log.pop_back();
		if ((color && material >= bestChoice) || (!color && material <= bestChoice)) {
			if (depth == 2) {
				//std::cout << "Prune at level 2" << std::endl;
			}
			break;
		}
	}
	return bestMat;
}

void take_move(Game &game) {
	std::vector<move_info> moves = game.get_all_moves(game.turn);
	move_info choice = moves[0];

	move_info log = game.try_move(moves[0].piece, moves[0].to);
	game.move_log.push_back(log);

	int bestMat = minimax(game, 2, !game.turn, game.turn ? -2000 : 2000);

	game.move_back(log);
	game.move_log.pop_back();
	
	for (int i = 1; i < moves.size(); i++) {
		log = game.try_move(moves[i].piece, moves[i].to);
		game.move_log.push_back(log);

		int material = minimax(game, 2, !game.turn, game.turn ? -2000 : 2000);

		game.move_back(log);
		game.move_log.pop_back();

		if ((game.turn && material > bestMat) || (!game.turn && material < bestMat)) {
			bestMat = material;
			choice = moves[i];
		}
	}
	game.log_move(choice.piece, choice.to);
}

void run_minimax(int, Game game, move_info move, std::mutex* writeLock, int* bestMat, move_info* choice) {
	//move.piece = game.board[move.from.y][move.from.x];

	move_info log = game.try_move(game.board[move.from.y][move.from.x], move.to);
	game.move_log.push_back(log);

	int material;
	std::string winner = game.check_for_winner(game.turn);
	if (winner != "") {
		if (winner == "Stalemate") {
			material = game.turn ? -100 : 100;
		}
		else{
			material = game.turn ? 10000 : -10000;
		}
	}
	else {
		writeLock->lock();
		int cutOff = *bestMat;
		writeLock->unlock();

		material = minimax(game, 2, !game.turn, cutOff);
	}

	writeLock->lock();
	if ((game.turn && material > *bestMat) || (!game.turn && material < *bestMat)) {
		*bestMat = material;
		*choice = log;
	}
	writeLock->unlock();
	std::cout << material << std::endl;

	game.move_back(log);
	game.move_log.pop_back();

}


void take_move_fast(Game& game) {
	std::vector<move_info> moves = game.get_all_moves(game.turn);
	int bestMat = game.turn ? -2000 : 2000;
	move_info choice = move_info{ {-1, -1}, {-1, -1}, NULL, NULL, false };

	std::mutex writeLock;
	ctpl::thread_pool p(16); /* sixteen threads in the pool */
	std::vector<std::future<void>> results(moves.size());

	for (int i = 0; i < moves.size(); ++i){
		results[i] = p.push(run_minimax, game, moves[i], &writeLock, &bestMat, &choice);
	}

	for (int i = 0; i < moves.size(); ++i) {
		results[i].get();
	}

	std::cout << "Max: " << bestMat << std::endl;
	std::cout << "[" << choice.from.x << ", " << choice.from.y << "] [" << choice.to.x << ", " << choice.to.y << "]" << choice.piece->piece << std::endl;
	game.log_move(game.board[choice.from.y][choice.from.x], choice.to);
}