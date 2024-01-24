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

int max_depth = MAX_DEPTH;
bool use_null = false;
bool use_LMR = true;
bool use_quiesce = true;
bool use_transposition = true;
bool extend_eval = true;

// Global Data collection variables
int node_count = 0;

// Transposition Table
typedef oneapi::tbb::concurrent_hash_map<std::string, std::array<int, 3>> StringTable;
StringTable transposition_table;
std::mutex table_lock;

// Timer
volatile bool time_up = false;
void start_timer(int seconds)
{
	std::this_thread::sleep_for(std::chrono::seconds(seconds));
	time_up = true;
}

// Midstate evalutation function
int heuristic(Game &game)
{
	int material = 0;
	// Lookup table
	int worth[] = {100, 320, 330, 500, 900, 0};

	// White Material
	for (auto piece : game.white_pieces)
	{
		if (piece.white >= 0)
		{
			material += worth[piece.type];
		}
	}
	// Black Material
	for (auto piece : game.black_pieces)
	{
		if (piece.white >= 0)
		{
			material -= worth[piece.type];
		}
	}

	if (!extend_eval)
	{
		return material;
	}

	// Pawn penalties
	material -= (game.count_bid_pawns(true) - game.count_bid_pawns(false)) * 10;

	// Passed Pawn
	material += (game.count_passed_pawns(true) - game.count_passed_pawns(false)) * 10;

	// Mobility
	material += (game.get_all_moves(game.turn).size() - game.get_all_moves(!game.turn).size()) * 4;

	// Bishop Pair
	if (game.white_pieces[10].white > -1 && game.white_pieces[11].white > -1)
		material += 30;
	if (game.black_pieces[10].white > -1 && game.black_pieces[11].white > -1)
		material -= 30;

	// Rook Attack
	material += (game.count_rook_attacks(true) - game.count_rook_attacks(false)) * 5;

	// Castling
	if (game.has_castled & 0b10)
		material += 20;
	if (game.has_castled & 0b01)
		material -= 20;

	return game.turn ? material : -material;
}

int utility(int status, bool color, int depth)
{
	// Color signifies who just took their move

	// Stalemate
	if (status == -1)
	{
		return -20000;
	}
	// Win
	return 400000 - depth;
}

void sort_moves_captured(std::vector<Move> &moves, Game &game)
{
	bool sort_needed = false;
	std::vector<int> scores(moves.size());
	for (unsigned int i = 0; i < moves.size(); i++)
	{
		Piece *piece = game.board[moves[i].to.y][moves[i].to.x];
		if (piece)
		{
			sort_needed = true;
			scores[i] = piece->type;
		}
		else
		{
			scores[i] = -1;
		}
	}
	if (sort_needed)
	{
		int j, key;
		for (unsigned int i = 1; i < scores.size(); i++)
		{
			key = scores[i];
			Move value = moves[i];
			j = i - 1;

			while (j >= 0 && scores[j] < key)
			{
				scores[j + 1] = scores[j];
				moves[j + 1] = moves[j];
				j = j - 1;
			}

			Piece *pieceI = game.board[moves[i].from.y][moves[i].from.x];
			while (j >= 0 && key == scores[j] && key != 0) {
				Piece *pieceJ = game.board[moves[j].from.y][moves[j].from.x];
				if (pieceJ->type <= pieceI->type)
				{
					break;
				}
				scores[j + 1] = scores[j];
				moves[j + 1] = moves[j];
				j = j - 1;
			}
			scores[j + 1] = key;
			moves[j + 1] = value;
		}
	}
}

void sort_moves_history(std::vector<Move> &moves, Game &game)
{
	int sort_needed = 0;
	// Initialize scores
	std::vector<int> scores;
	for (auto move : moves)
	{
		MoveDetails details = game.move(game.board[move.from.y][move.from.x], move.to);
		game.add_to_log(details);
		int rating = INT_MIN;
		std::string game_string = game.to_string();
		StringTable::const_accessor a;
		if (transposition_table.find(a, game_string))
		{
			rating = a->second[2];
			sort_needed++;
		}
		a.release();
		scores.push_back(rating);
		game.move_back(details);
		game.move_log.pop_back();
	}
	if (sort_needed)
	{
		int j, key;
		for (unsigned int i = 1; i < scores.size(); i++)
		{
			key = scores[i];
			Move value = moves[i];
			j = i - 1;

			while (j >= 0 && scores[j] < key)
			{
				scores[j + 1] = scores[j];
				moves[j + 1] = moves[j];
				j = j - 1;
			}
			scores[j + 1] = key;
			moves[j + 1] = value;
		}
	}
}

int quiesce(Game &game, int alpha, int beta, int depth)
{
	// Get default value
	int stand_pat = heuristic(game);
	// Check beta
	if (stand_pat >= beta)
		return stand_pat;
	// Check alpha
	if (alpha < stand_pat)
	{
		alpha = stand_pat;
	}
	// Get all moves
	std::vector<Move> moves = game.get_all_moves(game.turn);

	// Sort moves by value of piece being captured
	sort_moves_captured(moves, game);

	// Loop through moves
	for (auto move : moves)
	{
		if (game.board[move.to.y][move.to.x])
		{
			// Move
			MoveDetails details = game.move(game.board[move.from.y][move.from.x], move.to);
			game.add_to_log(details);

			node_count++;

			// Continue the search
			game.turn = !game.turn;
			int score = -quiesce(game, -beta, -alpha, depth + 1);
			game.turn = !game.turn;

			// Move back
			game.move_back(details);
			game.move_log.pop_back();

			// Check beta
			if (score >= beta)
				return score;

			// Update alpha
			if (score > alpha)
				alpha = score;
		}
		// Don't search non-capture moves
		else
		{
			break;
		}
	}
	// alpha is the highest material found
	return alpha;
}

int minimax(Game &game, int depth, int alpha, int beta, Move *choice, bool verify)
{
	if (time_up)
	{
		return alpha;
	}

	MoveDetails details;

	// Get all possible moves
	std::vector<Move> moves = game.get_all_moves(game.turn);

	// Sort for better cutoffs
	sort_moves_captured(moves, game);
	if (use_transposition && max_depth - depth > 1)
	{
		sort_moves_history(moves, game);
	}

	int best_score = INT_MIN;
	// Iterate through moves
	for (auto current_move : moves)
	{
		node_count++;

		int score = INT_MAX;
		int i = 0;

		details = game.move(game.board[current_move.from.y][current_move.from.x], current_move.to);
		game.add_to_log(details);

		// Table lookup
		std::string game_string;
		if (use_transposition)
		{
			game_string = game.to_string();
			StringTable::const_accessor a;
			if (transposition_table.find(a, game_string))
			{
				if (a->second[0] == depth && a->second[1] == max_depth)
				{
					score = a->second[2];
				}
			}
		}

		bool fail_high = false;
		// Evaluate node
		if (score == INT_MAX)
		{
			// Check for termination
			int terminated = game.check_for_winner(game.turn);
			if (terminated)
			{
				score = utility(terminated, game.turn, depth);
			}
			// Check for depth limit
			else if (depth >= max_depth)
			{
				if (use_quiesce)
				{
					game.turn = !game.turn;
					score = -quiesce(game, -beta, -alpha, depth + 1);
					game.turn = !game.turn;
				}
				else
				{
					score = heuristic(game);
				}
			}
			// Search
			else
			{
				// Null Search
				if (use_null && !game.check_for_check(game.turn) && (!verify || depth < max_depth - 1))
				{
					/* null-move search with minimal window around beta */
					// int value = -minimax(game, depth+R+1,-beta, -beta+1, NULL, verify);
					int value = minimax(game, depth + R + 1, beta - 1, beta, NULL, verify);
					if (value >= beta)
					{

						/* fail-high */
						if (verify)
						{
							/* reduce the depth by one ply */
							depth++;
							/* turn verification off for the sub-tree */
							verify = false;
							/* mark a fail-high flag, to detect zugzwangs later*/
							fail_high = true;
						}
						else
						{
							/* cutoff in a sub-tree with fail-high report */
							// Move back
							game.move_back(details);
							game.move_log.pop_back();

							return value;
						}
					}
				}
			research:
				// Search
				game.turn = !game.turn;
				if (!use_LMR || i == 0 || max_depth - depth < 3 || details.captured || game.check_for_check(game.turn))
				{
					// Search
					score = -minimax(game, depth + 1, -beta, -alpha, NULL, verify);
				}
				else
				{
					// Late Move Reduction
					score = -minimax(game, depth + 2, -beta, -alpha, NULL, verify);
					if (score > alpha)
					{
						score = -minimax(game, depth + 1, -beta, -alpha, NULL, verify);
					}
				}
				game.turn = !game.turn;

				// Verify
				if (fail_high && score < beta)
				{
					depth--;
					fail_high = false;
					verify = true;
					goto research;
				}
			}
			// Store result
			if (use_transposition)
			{
				StringTable::accessor a;
				if (transposition_table.insert(a, game_string))
				{
					a->second = {depth, max_depth, score};
				}
			}
		}

		// Move back
		game.move_back(details);
		game.move_log.pop_back();

		// Set best score variable for pruning
		if (score >= beta)
		{
			return score;
		}
		if (score > best_score)
		{
			best_score = score;
			if (depth == 1)
			{
				*choice = details.move;
			}
			if (score > alpha)
			{
				alpha = score;
			}
		}
		i++;
	}

	return best_score;
}

Move call_minimax(Game &game)
{
	Move choice;
	minimax(game, 1, -INT_MAX, INT_MAX, &choice, true);
	// Data collection
	printf("%d, ", node_count);
	node_count = 0;
	return choice;
}

void thread_func_minimax(int, Game &game, Move move, std::mutex *write_lock, int *alpha, int *beta, Move *choice)
{
	Game copy = game;
	// Try move - pointer to piece no longer valide, must use positions
	MoveDetails details = copy.move(copy.board[move.from.y][move.from.x], move.to);
	game.add_to_log(details);

	int material;
	// Check for termination
	int terminated = copy.check_for_winner(copy.turn);
	if (terminated)
	{
		material = utility(terminated, copy.turn, 1);
	}
	// Check for depth limit
	else if (max_depth == 1)
	{
		material = heuristic(copy);
	}
	else
	{
		// Get updated cutoff to reflect current search progress
		write_lock->lock();
		int a = *alpha;
		int b = *beta;
		write_lock->unlock();
		// Search
		copy.turn = !copy.turn;
		material = -minimax(copy, 2, -b, -a, NULL, true);
	}
	if (use_transposition)
	{
		StringTable::accessor a;
		if (transposition_table.insert(a, game.to_string()))
		{
			a->second = {1, max_depth, material};
		}
		a.release();
	}

	// Update choice if we have a better move
	write_lock->lock();

	// Set best material variable for pruning
	if (material > *alpha)
	{
		*alpha = material;
		*choice = details.move;
	}
	write_lock->unlock();
}

Move call_minimax_fast(Game &game)
{
	std::vector<Move> moves = game.get_all_moves(game.turn);
	int alpha = -INT_MAX;
	int beta = INT_MAX;
	Move choice;

	sort_moves_captured(moves, game);
	if (use_transposition)
	{
		sort_moves_history(moves, game);
	}

	std::mutex write_lock;
	ctpl::thread_pool p(16); /* sixteen threads in the pool */
	std::vector<std::future<void>> results(moves.size());

	for (unsigned int i = 0; i < moves.size(); ++i)
	{
		results[i] = p.push(thread_func_minimax, game, moves[i], &write_lock, &alpha, &beta, &choice);
	}

	for (unsigned int i = 0; i < moves.size(); ++i)
	{
		results[i].get();
	}

	return choice;
}

Move call_minimax_IDS(Game &game)
{
	Move move;
	if (TIMER)
	{
		time_up = false;
		std::thread timerThread(start_timer, MAX_RUN_TIME);
		max_depth = 0;
		Move temp_choice;
		while (!time_up)
		{
			max_depth++;
			move = temp_choice;
			minimax(game, 1, -INT_MAX, INT_MAX, &temp_choice, true);
		}
		// fprintf(stderr, "Max depth: %d\n", max_depth);
		timerThread.join();
		time_up = false;
	}
	else
	{
		max_depth = 1;
		for (int i = 0; i < MAX_DEPTH; i++)
		{
			minimax(game, 1, -INT_MAX, INT_MAX, &move, true);
			max_depth++;
		}
		max_depth = MAX_DEPTH;
	}

	// Disable transposition and empty the transpostion table
	clear_table();

	return move;
}

Move call_minimax_IDS_fast(Game &game)
{
	// Enable transposition and empty the transpostion table
	use_transposition = true;
	clear_table();

	Move move;
	if (TIMER)
	{
		time_up = false;
		std::thread timerThread(start_timer, MAX_RUN_TIME);

		max_depth = 0;
		Move temp_choice;
		while (!time_up)
		{
			max_depth++;
			move = temp_choice;
			temp_choice = call_minimax_fast(game);
		}
		// fprintf(stderr, "Max depth: %d\n", max_depth);
		timerThread.join();
		time_up = false;
	}
	else
	{
		max_depth = 0;
		for (int i = 1; i <= MAX_DEPTH; i += 1)
		{
			max_depth++;
			move = call_minimax_fast(game);
		}
	}
	printf("IDS max_depth: %d\n", max_depth);
	max_depth = MAX_DEPTH;

	// printf("IDS: %s\n", move.to_string().c_str());

	// Disable transposition and empty the transpostion table
	clear_table();
	use_transposition = false;

	return move;
}

Move move_with_opening(Game &game, Move (*func)(Game &))
{
	std::string book_move;
	std::string move_history = "";
	// Build move history string
	for (auto entry : game.move_log)
	{
		move_history += entry + " ";
	}
	// Search for a response in the opening database
	if (game.move_log.size() < MAXLENGTH &&
		lookup_move(move_history, book_move))
	{
		return Move().translate(book_move);
	}
	// If the game is out of book, call the continuation function
	return func(game);
}

void clear_table()
{
	transposition_table.clear();
}

int evaluate_board(Game &game)
{
	return quiesce(game, -INT_MAX, INT_MAX, 1);
}

int main()
{
	std::string line;
	while (std::getline(std::cin, line))
	{
		Game state;

		std::istringstream iss(line);

		// Temporary variable to store each word
		std::string word;

		// Read words using a loop
		while (iss >> word)
		{
			// Process or store each word as needed
			Move input_move;
			input_move.translate(word);
			state.log_move(input_move);
			state.switch_turns();
		}

		Move choice = move_with_opening(state, call_minimax_IDS);
		std::cout << choice.to_string() << std::endl;
	}
}