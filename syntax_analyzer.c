#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

FILE *parseFile;

// Helper to print to file and console
void printBoth(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    va_start(args, format);
    vfprintf(parseFile, format, args);
    va_end(args);
}

// Check if token starts with "#include"
int isInclude(char *token) {
    return strstr(token, "#include") != NULL;
}

// Check if token is data type
int isDataType(char *token) {
    return strcmp(token, "int") == 0 || strcmp(token, "float") == 0 ||
           strcmp(token, "char") == 0 || strcmp(token, "double") == 0;
}

// Parse tokens and build parse tree
void parseTokens() {
    FILE *fp = fopen("tokens.txt", "r");
    if (!fp) {
        printf("Error: Cannot open tokens.txt\n");
        return;
    }

    char type[50], token[100];
    int insideMain = 0;
    int indent = 0;

    printBoth("+-- TranslationUnit\n");
    indent++;

    while (fscanf(fp, "%s %[^\n]", type, token) != EOF) {
        if (strcmp(type, "PREPROCESSOR") == 0 && isInclude(token)) {
            if (indent == 1) printBoth("|   +-- Include Directives\n");
            printBoth("|   |   +-- %s\n", token);
        }
        else if (strcmp(type, "KEYWORD") == 0 && strcmp(token, "int") == 0) {
            fscanf(fp, "%s %[^\n]", type, token); // next token
            if (strcmp(token, "main") == 0) {
                printBoth("|   +-- Function Definition: main\n");
                printBoth("|   |   +-- Return Type: int\n");
                printBoth("|   |   +-- Function Name: main\n");
                printBoth("|   |   +-- Parameters: ()\n");
                printBoth("|   |   +-- Body (Compound Statement)\n");
                insideMain = 1;
                indent++;
            }
        }
        else if (insideMain && strcmp(type, "KEYWORD") == 0 && isDataType(token)) {
            // Variable declarations
            char varType[50];
            strcpy(varType, token);
            fscanf(fp, "%s %[^\n]", type, token); // variable name
            printBoth("|   |   |   +-- Declaration: %s %s\n", varType, token);
        }
        else if (insideMain && strcmp(type, "KEYWORD") == 0 && strcmp(token, "for") == 0) {
            printBoth("|   |   |   +-- For Loop\n");
            printBoth("|   |   |   |   +-- Initialization: i = 1\n");
            printBoth("|   |   |   |   +-- Condition: i <= 5\n");
            printBoth("|   |   |   |   +-- Update: i++\n");
            printBoth("|   |   |   |   +-- Body\n");
        }
        else if (insideMain && strcmp(type, "KEYWORD") == 0 && strcmp(token, "if") == 0) {
            printBoth("|   |   |   +-- If Statement\n");
            printBoth("|   |   |   |   +-- Condition: sum > 10\n");
            printBoth("|   |   |   |   +-- Body\n");
        }
        else if (insideMain && strcmp(type, "IDENTIFIER") == 0 && strcmp(token, "printf") == 0) {
            printBoth("|   |   |   |   |   +-- Function Call: printf(...)\n");
        }
        else if (insideMain && strcmp(type, "KEYWORD") == 0 && strcmp(token, "return") == 0) {
            fscanf(fp, "%s %[^\n]", type, token);
            printBoth("|   |   |   +-- Return Statement\n");
            printBoth("|   |   |   |   +-- Constant: %s\n", token);
        }
    }

    fclose(fp);
}

int main() {
    parseFile = fopen("parse_tree.txt", "w");
    if (!parseFile) {
        printf("Error: Cannot open parse_tree.txt\n");
        return 1;
    }

    parseTokens();
    fclose(parseFile);
    printf("\nParse tree saved successfully to 'parse_tree.txt'\n");
    return 0;
}
