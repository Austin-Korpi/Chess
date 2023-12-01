#include "MTD.h"
#include "Engine.h"
#include <climits>
#include <cstring>
#include <chrono>

extern bool useTransposition;
extern int maxdepth;

int MTD(Game &game, int first, Move* choice)
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
        g = minimax(game, 1, bound - 1, bound, choice, true);
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

Move call_MTD(Game& game) {
    Move choice;
    useTransposition = true;
    clearTable();

    MTD(game, 0, &choice);
    printStats();
    // printf("MTD: %s\n", choice.toString().c_str());

    useTransposition = false;
    return choice;
}

Move call_MTD_IDS(Game& game) {
    Move choice;
    useTransposition = true;
    clearTable();

    auto start = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(start - start);

    int estimate = 0;
    maxdepth = 0;
    while (duration.count() < 1000) {
        maxdepth++;
        estimate = MTD(game, estimate, &choice);
        auto end = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    }
    // for (maxdepth = 1; maxdepth <= MAXDEPTH; maxdepth++) {
    //     estimate = MTD(game, estimate, &choice);
    // }
    printStats();
    // printf("MTD_IDS: %s\n", choice.toString().c_str());
    printf("MTD maxdepth: %d\n", maxdepth);

    maxdepth = MAXDEPTH;
    useTransposition = false;
    return choice;
}

