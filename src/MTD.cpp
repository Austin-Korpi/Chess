#include "MTD.h"
#include "Engine.h"
#include <climits>
#include <cstring>
#include <chrono>
#include <thread>

extern bool useTransposition;
extern bool useLMR;
extern bool useQuiesce;
extern int maxdepth;
extern volatile bool timeUp;

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
    useTransposition = true;
    clearTable();

    MTD(game, 0, &choice);
    printStats();

    useTransposition = false;
    return choice;
}

Move call_MTD_IDS(Game &game)
{
    Move choice;
    useTransposition = true;
    useLMR = false;
    useQuiesce = false;
    timeUp = false;
    clearTable();

    int estimate = 0;
    maxdepth = 0;

    // std::thread timerThread(waitForTimeAndChangeVariable, MAX_RUN_TIME);
    // while (!timeUp) {
    for (int i = 0; i < MAXDEPTH; i++)
    {
        maxdepth++;
        estimate = MTD(game, estimate, &choice);
    }
    // printStats();
    // printf("MTD maxdepth: %d, %s\n", maxdepth, choice.toString().c_str());

    // timerThread.join();
    clearTable();
    maxdepth = MAXDEPTH;
    useTransposition = false;
    return choice;
}
