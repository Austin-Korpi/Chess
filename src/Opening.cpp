#include <algorithm>
#include <string.h>

#include "Opening.h"

#define DATABASE "OpeningDatabase.txt"

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

        OpeningNode* current = openingTree;
        while (token != NULL) {
            // Initialize the tree
            if (current == NULL) {
                openingTree = new OpeningNode(token);
                current = openingTree;
            } else {
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
            }
            token = strtok(NULL, " \t\n");
        }
    }

    fclose(file); // Close the file when done

    return 0;
}

bool lookupMove(char* moves, std::string& move) {
    if (!openingTree) {
        buildOpeningTree();
    }
    
    char *token = strtok(moves, " \t\n"); // Tokenize the line into words
    OpeningNode* current = openingTree;
    while (token != NULL) {
        OpeningNode* next = current->next(token);
        if (next == NULL) {
            return false;
        }
        char *token = strtok(moves, " \t\n"); // Tokenize the line into words
    }
    if (current->children.size() == 0) {
        return false;
    }

    move = current->children[0]->name;
    return true;
}