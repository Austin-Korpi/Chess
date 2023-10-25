#include "MCTS.h"
#include <math.h>

#define REPETITIONS 1000

move_info MCNode::getBestMove()
{
    int max = -1;
    move_info bestMove;
    for (auto child : children)
    {
        move_info move = child->state.move_log.back();
        printf("%s %f/%d\n", move.toString().c_str(), child->wins, child->visited);
        if (child->visited > max)
        {
            max = child->visited;
            bestMove = move;
        }
    }
    printf("\n");
    return bestMove;
}

MCNode *MCNode::select()
{
    // If leaf node
    if (moves.size() > 0)
    {
        // Add new node to tree and return it
        children.push_back(new MCNode(state, moves.back(), this));
        moves.pop_back();
        return children.back();
    }
    // return this node if no children are present
    MCNode *choice = this;
    // find the child with the largest uct score
    float max = 0;
    for (auto child : children)
    {
        float uct = child->wins / child->visited + C * sqrt(log(visited) / child->visited);
        if (uct > max)
        {
            max = uct;
            choice = child;
        }
    }
    return choice;
}

void MCNode::expand()
{
    moves = state.get_all_moves(state.turn);
}

int MCNode::simulate()
{
    while (true)
    {
        move_info move = state.get_random_move(state.turn);
        state.log_move(move.piece, move.to);
        std::string result = state.switch_turns();
        if (result != "")
        {
            if (result == "Stalemate")
            {
                return 0;
            }
            else
            {
                return state.turn ? -1 : 1;
            }
        }
    }
}

void MCNode::backpropogate(int result)
{
    visited++;
    if (result == 0)
    {
        wins += 0.5;
    }
    else if ((result == -1 && state.turn == false) || (result == 1 && state.turn == true))
    {
        wins += 1;
    }
    if (parent)
    {
        parent->backpropogate(result);
    }
}

// Root contructor
MCNode::MCNode(Game game) : state(game)
{
    // The root is not a leaf node
    expand();
}

// Global data collection variables
int unvisitedNodes;
int visitedNodes;

MCNode::~MCNode()
{
    for (auto child : children)
    {
        if (child->visited == 0)
            unvisitedNodes++;
        else
            visitedNodes++;

        delete child;
    }
}

// Child constructor
MCNode::MCNode(Game game, move_info &move, MCNode *parent) : state(game)
{
    this->parent = parent;
    state.log_move(state.board[move.from.y][move.from.x], move.to);
    if (state.switch_turns() != "")
    {
        // TODO propogate so that we never come back here
        terminal = true;
    }
}

move_info monte_carlo_tree_search(Game &game)
{
    unvisitedNodes = 0;
    visitedNodes = 0;
    move_info move;
    {
        // Put the current game state into the tree
        MCNode root = MCNode(game);
        int i;
        for (i = 0; i < REPETITIONS; i++)
        {
            // Chose one 'leaf' (not maxed out node) from the current tree using UCT
            MCNode *leaf = root.select();
            // Add one leaf's children to the tree
            leaf->expand();
            // Simulate the game from the child
            leaf = leaf->select();
            int result = leaf->simulate();
            // Backpropogation
            leaf->backpropogate(result);
        }
        move = root.getBestMove();
    }

    return move;
}

void monte_carlo(Game &game)
{
    move_info move = monte_carlo_tree_search(game);
    if (!game.log_move(game.board[move.from.y][move.from.x], move.to)) {
        printf("Illegal move: %s\n", move.toString().c_str());
        monte_carlo(game);
    }
}