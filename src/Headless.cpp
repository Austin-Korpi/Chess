#include <iostream>
#include <chrono>

#include "MCTS.h"
#include "Pieces.h"
#include "Game.h"
#include "Engine.h"
#include "MTD.h"

int main() {
    for(int numGames = 0; numGames < 1; numGames++) {
        Game game = Game();
        std::string winner = "";
        while (winner == "") {
            Move choice;
            
            extern bool null_ok;
            extern bool useLMR;
            extern bool useQuiesce;
            extern bool useTransposition;
            extern bool useSort;
            extern int nodeCount;

            // No IDS
            choice = move_with_opening(game, &call_minimax);
            printf("%d, ", nodeCount);
            nodeCount = 0;
            // MTD(f) IDS
            move_with_opening(game, &call_MTD_IDS);
            printf("%d, ", nodeCount);
            nodeCount = 0;
            // Move Ordering
            useSort = true;
            move_with_opening(game, &call_minimax);
            printf("%d, ", nodeCount);
            nodeCount = 0;
            useSort = false;
            // TT
            useTransposition = true;
            move_with_opening(game, &call_minimax);
            printf("%d, ", nodeCount);
            nodeCount = 0;
            // TT sort
            useSort = true;
            move_with_opening(game, &call_minimax_IDS);
            printf("%d, ", nodeCount);
            nodeCount = 0;
            // LMR
            useLMR = true;
            move_with_opening(game, &call_minimax_IDS);
            printf("%d, ", nodeCount);
            nodeCount = 0;
            useLMR = false;
            useSort = false;
            useTransposition = false;
            // NULL
            null_ok = true;
            move_with_opening(game, &call_minimax);
            printf("%d, ", nodeCount);
            nodeCount = 0;
            null_ok = false;
            // Quiescent
            useQuiesce = true;
            move_with_opening(game, &call_minimax);
            printf("%d, ", nodeCount);
            nodeCount = 0;
            useQuiesce = false;
            
            game.log_move(choice);
            printf("\n");
            winner = game.switch_turns();
        }    
        std::cout << winner << std::endl;    
    }
}