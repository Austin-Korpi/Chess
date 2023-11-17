#include "Game.h"

#define C sqrt(2)

class MCNode
{
    Game state;
    std::vector<Move> moves;
    MCNode *parent = NULL;
    std::vector<MCNode *> children;
    int visited = 0;
    float wins = 0;
    bool terminal = false;

public:
    Move getBestMove();
    MCNode *select();
    void expand();
    int simulate();
    void backpropogate(int result);
    // Root contructor
    MCNode(Game game);
    // Child constructor
    MCNode(Game game, Move &move, MCNode *parent);
    ~MCNode();
};

Move monte_carlo_tree_search(Game &game);
Move monte_carlo(Game &game);
