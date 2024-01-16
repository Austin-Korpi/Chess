#include "MTD.h"
#include "Engine.h"
#include <climits>
#include <cstring>
#include <chrono>
#include <thread>

extern bool use_transposition;
extern bool use_LMR;
extern bool use_quiesce;
extern int max_depth;
extern volatile bool time_up;

int MTD(Game &game, int first, Move *choice)
{
    int g;
    int upper = INT_MAX;
    int lower = -INT_MAX;
    int bound = first;

    if (bound == lower)
    {
        bound++;
    }

    while (upper != lower)
    {
        g = minimax(game, 1, bound - 1, bound, choice, true);

        if (g < bound)
        {
            upper = g;
            bound = g;
        }
        else
        {
            lower = g;
            bound = g + 1;
        }
    }
    return upper;
}

Move call_MTD(Game &game)
{
    Move choice;
    use_transposition = true;
    clear_table();

    MTD(game, 0, &choice);

    use_transposition = false;
    return choice;
}

Move call_MTD_IDS(Game &game)
{
    Move choice;
    use_transposition = true;
    use_LMR = false;
    use_quiesce = false;
    time_up = false;
    clear_table();

    int estimate = 0;
    max_depth = 0;

    for (int i = 0; i < MAX_DEPTH; i++)
    {
        max_depth++;
        estimate = MTD(game, estimate, &choice);
    }

    clear_table();
    max_depth = MAX_DEPTH;
    use_transposition = false;
    return choice;
}
