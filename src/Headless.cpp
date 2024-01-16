#include <iostream>
#include <chrono>
#include <sys/wait.h>
#include <unistd.h>
#include <cstring>

#include "Pieces.h"
#include "Game.h"

void init_engine(FILE *&engine_in, FILE *&engine_out)
{
    pid_t child_pid;
    int pipe_in[2];
    int pipe_out[2];

    // Create a pipe
    if (pipe(pipe_in) == -1)
    {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // Create a pipe
    if (pipe(pipe_out) == -1)
    {
        perror("pipe");
        close(pipe_in[0]);
        close(pipe_out[1]);
        exit(EXIT_FAILURE);
    }

    // Create a child process
    if ((child_pid = fork()) == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (child_pid == 0)
    {
        // Child process

        // Redirect the standard input to the read end of the pipe
        dup2(pipe_in[0], STDIN_FILENO);

        close(pipe_in[1]);

        close(pipe_out[0]);

        // Redirect the standard output to the write end of the pipe
        dup2(pipe_out[1], STDOUT_FILENO);

        // Execute a command in the child process
        execl("./engine.app", "./engine.app", NULL);

        // If execl fails
        perror("execl");
        exit(EXIT_FAILURE);
    }
    else
    {
        close(pipe_in[0]);
        close(pipe_out[1]);
        engine_in = fdopen(pipe_in[1], "w");
        engine_out = fdopen(pipe_out[0], "r");
    }
}

void init_SF(FILE *&SF_in, FILE *&SF_out)
{
    pid_t child_pid;
    int pipe_in[2];
    int pipe_out[2];

    // Create a pipe
    if (pipe(pipe_in) == -1)
    {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // Create a pipe
    if (pipe(pipe_out) == -1)
    {
        perror("pipe");
        close(pipe_in[0]);
        close(pipe_out[1]);
        exit(EXIT_FAILURE);
    }

    // Create a child process
    if ((child_pid = fork()) == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (child_pid == 0)
    {
        // Child process

        // Redirect the standard input to the read end of the pipe
        dup2(pipe_in[0], STDIN_FILENO);

        close(pipe_in[1]);

        close(pipe_out[0]);

        // Redirect the standard output to the write end of the pipe
        dup2(pipe_out[1], STDOUT_FILENO);

        // Execute a command in the child process
        execl("./stockfish-windows-x86-64-avx2.exe", "./stockfish-windows-x86-64-avx2.exe", NULL);

        // If execl fails
        perror("execl");
        exit(EXIT_FAILURE);
    }
    else
    {
        close(pipe_in[0]);
        close(pipe_out[1]);
        SF_in = fdopen(pipe_in[1], "w");
        SF_out = fdopen(pipe_out[0], "r");

        const char *msg = "setoption name Use NNUE value false\n";
        fwrite(msg, 1, strlen(msg), SF_in);
        fflush(SF_in);
    }
}

Move get_engine_move(Game &game, FILE *engine_in, FILE *engine_out)
{
    // Get string with all game moves
    std::string moves = "";
    for (int i = 0; i < (int)game.move_log.size(); i++)
    {
        moves += game.move_log[i] + " ";
    }
    moves += '\n';

    // Write string to engine
    if (fwrite(moves.c_str(), 1, strlen(moves.c_str()), engine_in) < strlen(moves.c_str()))
    {
        perror("fwrite");
    }
    fflush(engine_in);

    // Read move
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), engine_out) != nullptr)
    {
        if (strlen(buffer) == 5)
        {
            break;
        }
    }
    buffer[4] = 0;

    // Convert move
    Move choice;
    choice.translate(std::string(buffer));

    return choice;
}

Move get_SF_move(Game &game, FILE *SF_in, FILE *SF_out)
{
    // Get string with all game moves
    std::string moves = "position startpos moves ";
    for (int i = 0; i < (int)game.move_log.size(); i++)
    {
        moves += game.move_log[i] + " ";
    }
    moves += "\ngo depth 5\n";

    // Write string to engine
    if (fwrite(moves.c_str(), 1, strlen(moves.c_str()), SF_in) < strlen(moves.c_str()))
    {
        perror("fwrite");
    }
    fflush(SF_in);

    // Read move
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), SF_out) != nullptr)
    {
        if (strstr(buffer, "bestmove") != NULL)
        {
            break;
        }
    }
    buffer[13] = 0;

    // Convert move
    Move choice;
    choice.translate(std::string(&buffer[9]));

    return choice;
}

int main()
{
    // Initialize AI
    FILE *engine_in;
    FILE *engine_out;
    init_engine(engine_in, engine_out);
    std::cout << "Engine started" << std::endl;

    // Initialize Stockfish
    FILE *SF_in;
    FILE *SF_out;
    init_SF(SF_in, SF_out);
    std::cout << "StockFish started" << std::endl;

    for (int num_games = 0; num_games < 20; num_games++)
    {
        Game game = Game();

        std::string winner = "";
        while (winner == "")
        {
            Move choice;

            // My AI
            if (game.turn)
            {
                choice = get_engine_move(game, engine_in, engine_out);
            }
            // StockFish
            else
            {
                choice = get_SF_move(game, SF_in, SF_out);
            }

            // std::cout << choice.to_string() << std::endl;
            game.log_move(choice);

            winner = game.switch_turns();
        }
        std::cout << winner << std::endl;
    }
}