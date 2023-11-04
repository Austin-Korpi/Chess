#include <vector>
#include <string>

#define DATABASE "OpeningDatabase.txt"
#define MAXDEPTH 36

struct OpeningNode{
    std::string name;
    std::vector<OpeningNode*> children;

    OpeningNode(char* token): name(token), children(){}
    OpeningNode* next(char* token);
};

int buildOpeningTree();

bool lookupMove(char* moves, std::string& move);