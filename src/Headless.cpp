#include <iostream>
#include <chrono>

#include "MCTS.h"
#include "Pieces.h"
#include "Game.h"
#include "Engine.h"
#include "MTD.h"

int main() {
    Game game = Game();
    std::string winner = "";
    while (winner == "") {
        if (game.turn == false) {
            printf("\n--Black Move--\n");
            auto start = std::chrono::high_resolution_clock::now();

            game.log_move(move_with_opening(game, &call_MTD_IDS));

            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            std::cout << "Variable MTD: " << duration.count() << " miliseconds" << std::endl;
            
        } else {
            printf("\n--White Move--\n");
            auto start = std::chrono::high_resolution_clock::now();
            
            game.log_move(move_with_opening(game, &call_minimax_IDS_fast));

            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            std::cout << "Time: " << duration.count() << " miliseconds" << std::endl;
        }
        winner = game.switch_turns();
    }        
    std::cout << "Winner: " << winner << std::endl;    
}