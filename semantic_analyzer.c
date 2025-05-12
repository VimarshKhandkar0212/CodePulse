#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX 100

typedef struct {
    char type[30];
    char value[100];
} Token;

typedef struct {
    char name[100];
    char type[30]; // e.g., int, char, float
} Symbol;

Token tokens[MAX];
Symbol symbolTable[MAX];
int pos = 0, total = 0, symCount = 0;

// Load tokens from lexer output
void loadTokens() {
    FILE *fp = fopen("tokens.txt", "r");
    if (!fp) {
        printf("Could not open tokens.txt\n");
        exit(1);
    }
    while (fscanf(fp, "%s %s", tokens[total].type, tokens[total].value) != EOF) {
        total++;
    }
    fclose(fp);
}

// Utility: Find symbol in table
int findSymbol(char *name) {
    for (int i = 0; i < symCount; i++) {
        if (strcmp(symbolTable[i].name, name) == 0)
            return i;
    }
    return -1;
}

// Add variable to symbol table
void addSymbol(char *name, char *type) {
    if (findSymbol(name) != -1) {
        printf("Semantic Error: Redeclaration of variable '%s'\n", name);
        exit(1);
    }
    strcpy(symbolTable[symCount].name, name);
    strcpy(symbolTable[symCount].type, type);
    symCount++;
}

// Match current token type
int match(char *expectedType) {
    if (pos < total && strcmp(tokens[pos].type, expectedType) == 0) {
        pos++;
        return 1;
    }
    return 0;
}

// Expect a token type and value
void expect(char *expectedType) {
    if (!match(expectedType)) {
        printf("Unexpected token %s (%s)\n", tokens[pos].type, tokens[pos].value);
        exit(1);
    }
}

// Handle variable declaration
void declaration() {
    char varType[30];
    strcpy(varType, tokens[pos].value); // e.g., int
    expect("KEYWORD");

    while (1) {
        if (!match("IDENTIFIER")) {
            printf("Semantic Error: Expected variable name after type\n");
            exit(1);
        }
        char varName[100];
        strcpy(varName, tokens[pos - 1].value);

        if (match("OPERATOR") && strcmp(tokens[pos - 1].value, "=") == 0) {
            if (match("NUMBER")) {
                if (strcmp(varType, "char") == 0) {
                    printf("Semantic Error: Cannot assign int to char variable '%s'\n", varName);
                    exit(1);
                }
            } else if (match("STRING_LITERAL")) {
                if (strcmp(varType, "int") == 0 || strcmp(varType, "float") == 0) {
                    printf("Semantic Error: Cannot assign string to %s variable '%s'\n", varType, varName);
                    exit(1);
                }
            } else if (match("IDENTIFIER")) {
                int idx = findSymbol(tokens[pos - 1].value);
                if (idx == -1) {
                    printf("Semantic Error: Variable '%s' used before declaration\n", tokens[pos - 1].value);
                    exit(1);
                }
                // Optionally check type compatibility
            } else {
                printf("Semantic Error: Invalid assignment to variable '%s'\n", varName);
                exit(1);
            }
        }

        addSymbol(varName, varType);

        if (match("SEPARATOR") && strcmp(tokens[pos - 1].value, ";") == 0) {
            break;
        } else if (match("SEPARATOR") && strcmp(tokens[pos - 1].value, ",") == 0) {
            continue;
        } else {
            printf("Semantic Error: Invalid declaration syntax for '%s'\n", varName);
            exit(1);
        }
    }
}

// Assignment check
void assignment() {
    if (!match("IDENTIFIER")) return;
    char varName[100];
    strcpy(varName, tokens[pos - 1].value);
    int idx = findSymbol(varName);
    if (idx == -1) {
        printf("Semantic Error: Variable '%s' used before declaration\n", varName);
        exit(1);
    }

    expect("OPERATOR"); // '='

    if (match("NUMBER")) {
        if (strcmp(symbolTable[idx].type, "char") == 0) {
            printf("Semantic Error: Cannot assign int to char variable '%s'\n", varName);
            exit(1);
        }
    } else if (match("STRING_LITERAL")) {
        if (strcmp(symbolTable[idx].type, "int") == 0 || strcmp(symbolTable[idx].type, "float") == 0) {
            printf("Semantic Error: Cannot assign string to %s variable '%s'\n", symbolTable[idx].type, varName);
            exit(1);
        }
    } else if (match("IDENTIFIER")) {
        int otherIdx = findSymbol(tokens[pos - 1].value);
        if (otherIdx == -1) {
            printf("Semantic Error: Variable '%s' used before declaration\n", tokens[pos - 1].value);
            exit(1);
        }
    } else {
        printf("Semantic Error: Invalid assignment to variable '%s'\n", varName);
        exit(1);
    }

    expect("SEPARATOR"); // ';'
}

// Main semantic analyzer
void analyze() {
    while (pos < total) {
        if (strcmp(tokens[pos].type, "KEYWORD") == 0 &&
           (strcmp(tokens[pos].value, "int") == 0 ||
            strcmp(tokens[pos].value, "char") == 0 ||
            strcmp(tokens[pos].value, "float") == 0)) {
            declaration();
        } else if (strcmp(tokens[pos].type, "IDENTIFIER") == 0) {
            assignment();
        } else {
            pos++;
        }
    }
    printf("Semantic Analysis Successful ✅\n");
}

int main() {
    loadTokens();
    analyze();
    return 0;
}
