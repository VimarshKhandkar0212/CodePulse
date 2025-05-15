#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TOKENS 10000

// Token structure
typedef struct {
    char type[32];
    char value[256];
} Token;

// Global variables
Token tokens[MAX_TOKENS];
int tokenCount = 0;
int currentIndex = 0;
FILE *treeFile;

// Read tokens from file
void readTokens(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        printf("Error opening token file.\n");
        exit(1);
    }

    while (fscanf(fp, "%s %s", tokens[tokenCount].type, tokens[tokenCount].value) != EOF) {
        tokenCount++;
    }

    fclose(fp);
}

// Write a node to the parse tree file
void writeNode(const char *label, const char *value) {
    if (value) {
        fprintf(treeFile, "%s (%s)\n", label, value);
        printf("%s (%s)\n", label, value);     // Print to console
    } else {
        fprintf(treeFile, "%s\n", label);
        printf("%s\n", label);                 // Print to console
    }
}

// Parse preprocessor directives
void parsePreprocessorDirectives() {
    while (currentIndex < tokenCount && strcmp(tokens[currentIndex].type, "PREPROCESSOR") == 0) {
        writeNode("|   +-- PreprocessorDirective", tokens[currentIndex].value);
        currentIndex++;
    }
}

// Parse a simple function definition
void parseFunctionDefinition() {
    if (strcmp(tokens[currentIndex].type, "KEYWORD") == 0 &&
        (strcmp(tokens[currentIndex].value, "int") == 0 ||
         strcmp(tokens[currentIndex].value, "float") == 0 ||
         strcmp(tokens[currentIndex].value, "void") == 0 ||
         strcmp(tokens[currentIndex].value, "char") == 0 ||
         strcmp(tokens[currentIndex].value, "double") == 0)) {

        writeNode("|   +-- FunctionDefinition", NULL);
        writeNode("|       +-- ReturnType", tokens[currentIndex].value);
        currentIndex++;

        if (strcmp(tokens[currentIndex].type, "IDENTIFIER") == 0) {
            writeNode("|       +-- FunctionName", tokens[currentIndex].value);
            currentIndex++;

            if (strcmp(tokens[currentIndex].value, "(") == 0) {
                writeNode("|       +-- ParameterListStart", tokens[currentIndex].value);
                currentIndex++;

                // Handle parameters (skip for now, or expand as needed)
                while (strcmp(tokens[currentIndex].value, ")") != 0 && currentIndex < tokenCount) {
                    currentIndex++;
                }

                if (strcmp(tokens[currentIndex].value, ")") == 0) {
                    writeNode("|       +-- ParameterListEnd", tokens[currentIndex].value);
                    currentIndex++;

                    if (strcmp(tokens[currentIndex].value, "{") == 0) {
                        writeNode("|       +-- FunctionBodyStart", tokens[currentIndex].value);
                        currentIndex++;

                        // Parse simple body (only basic statements)
                        while (strcmp(tokens[currentIndex].value, "}") != 0 && currentIndex < tokenCount) {
                            if (strcmp(tokens[currentIndex].type, "KEYWORD") == 0) {
                                writeNode("|           +-- Statement", tokens[currentIndex].value);
                            } else if (strcmp(tokens[currentIndex].type, "IDENTIFIER") == 0) {
                                writeNode("|           +-- Identifier", tokens[currentIndex].value);
                            } else if (strcmp(tokens[currentIndex].type, "OPERATOR") == 0) {
                                writeNode("|           +-- Operator", tokens[currentIndex].value);
                            } else if (strcmp(tokens[currentIndex].type, "NUMBER") == 0) {
                                writeNode("|           +-- Number", tokens[currentIndex].value);
                            } else if (strcmp(tokens[currentIndex].type, "SEPARATOR") == 0 &&
                                       strcmp(tokens[currentIndex].value, ";") == 0) {
                                writeNode("|           +-- Semicolon", tokens[currentIndex].value);
                            }
                            currentIndex++;
                        }

                        if (strcmp(tokens[currentIndex].value, "}") == 0) {
                            writeNode("|       +-- FunctionBodyEnd", tokens[currentIndex].value);
                            currentIndex++;
                        }
                    }
                }
            }
        }
    }
}

// Parse the full translation unit
void parseTranslationUnit() {
    writeNode("+-- TranslationUnit", NULL);
    parsePreprocessorDirectives();
    while (currentIndex < tokenCount) {
        parseFunctionDefinition();
    }
}

// Main function
int main() {
    treeFile = fopen("parse_tree.txt", "w");
    if (!treeFile) {
        printf("Error opening parse tree file.\n");
        return 1;
    }

    readTokens("tokens.txt");

    parseTranslationUnit();

    fclose(treeFile);
    printf("Syntax analysis complete. Parse tree written to 'parse_tree.txt'\n");
    return 0;
}
