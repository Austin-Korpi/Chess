#include <vector>
#include <string>

#define MAXLENGTH 36

struct OpeningNode
{
    std::string name;
    std::vector<OpeningNode *> children;

    OpeningNode(const char *token) : name(token), children() {}
    OpeningNode *next(char *token);
};

int build_opening_tree();

bool lookup_move(std::string moveHistory, std::string &move);