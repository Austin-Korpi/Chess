#include <algorithm>
#include <string.h>
#include <time.h>

#include "Opening.h"

#define DATABASE "resources/OpeningDatabase.txt"

OpeningNode* OpeningNode::next(char* token) {
    auto it = std::find_if(children.begin(),
                            children.end(),
                            [token](const OpeningNode* elem) {return elem->name == std::string(token);});
    if (it == children.end()) {
        return NULL;
    }
    return *it;   
}

// Global tree reference
OpeningNode* openingTree = NULL;

int buildOpeningTree() {
    FILE *file = fopen(DATABASE, "r");

    if (file == NULL) {
        fprintf(stderr, "Failed to open the file.\n");
        return 1;
    }

    char line[256]; 
    while (fgets(line, sizeof(line), file) != NULL) {
        char *token = strtok(line, " \t\n"); // Tokenize the line into words

        // Initialize the tree
        if (openingTree == NULL){
            openingTree = new OpeningNode("");
        }
        OpeningNode* current = openingTree;
        while (token != NULL) {
            // Search for the token
            OpeningNode* next = current->next(token);
                // Move to the next node if present already
            if (next == NULL) {
                current->children.push_back(new OpeningNode(token));
                current = current->children.back();
            } 
            // Add the node and move to it
            else {
                current = next;
            }
            // Get the next token
            token = strtok(NULL, " \t\n");
        }
    }

    fclose(file); // Close the file when done

    return 0;
}

bool lookupMove(std::string moveHistory, std::string& move) {
    if (!openingTree) {
        buildOpeningTree();
        srand(time(0));
    }

    char *moves = new char[moveHistory.length() + 1];
    strcpy(moves, moveHistory.c_str());
    
    char *token = strtok(moves, " \t\n"); // Tokenize the line into words
    OpeningNode* current = openingTree;
    while (token != NULL) {
        OpeningNode* next = current->next(token);
        if (next == NULL) {
            return false;
        }
        current = next;
        token = strtok(NULL, " \t\n"); // Tokenize the line into words
    }
    if (current->children.size() == 0) {
        return false;
    }

    delete moves;

    int i  = 0;
    int length = current->children.size();
    float denominator = (length*length - length) / 2 + length;
    float randomNumber = (float) rand() / RAND_MAX;
    float p = 0;
    for (i = length - 1; i >= 0; i--) {
        p += (length - i)/denominator;
        if (randomNumber < p) {
            break;
        }
    }
    move = current->children[i]->name;
    return true;
}