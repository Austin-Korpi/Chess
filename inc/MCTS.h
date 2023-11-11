#include "Game.h"

#define C sqrt(2)

class MCNode
{
    Game state;
    std::vector<move_info> moves;
    MCNode *parent = NULL;
    std::vector<MCNode *> children;
    int visited = 0;
    float wins = 0;
    bool terminal = false;

public:
    move_info getBestMove();
    MCNode *select();
    void expand();
    int simulate();
    void backpropogate(int result);
    // Root contructor
    MCNode(Game game);
    // Child constructor
    MCNode(Game game, move_info &move, MCNode *parent);
    ~MCNode();
};

move_info monte_carlo_tree_search(Game &game);
move_info monte_carlo(Game &game);
