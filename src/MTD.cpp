#include "MTD.h"
#include "Engine.h"
#include <climits>
#include <cstring>

extern bool useTransposition;
extern int maxdepth;

int MTD(Game &game, int first, move_info* choice)
{
    int g;
    int upper = INT_MAX;
    int lower = INT_MIN;
    int bound = first;

    if (bound == lower)
    {
        bound++;
    }

    while (upper != lower)
    {
        g = minimax(game, 1, bound - 1, bound, choice);
	    // printf("MTD result: %d\n", g);

        if (g < bound) {
            upper = g;
            bound = g;
        } else {
            lower = g;
            bound = g+1;
        }
    }
	// printf("MTD move: %s\n", choice->toString().c_str());
    return upper;
}

move_info call_MTD(Game& game) {
    move_info choice;
    useTransposition = true;
    clearTable();

    MTD(game, 0, &choice);
    printStats();

    useTransposition = false;
    return choice;
}

move_info call_MTD_IDS(Game& game) {
    move_info choice;
    useTransposition = true;
    clearTable();

    int estimate = 0;
    for (maxdepth = 1; maxdepth <= MAXDEPTH; maxdepth++) {
        estimate = MTD(game, estimate, &choice);
    }
    printStats();

    maxdepth = MAXDEPTH;
    useTransposition = false;
    return choice;
}

