#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SYMBOLS 1000

// Structure for a symbol table entry
typedef struct {
    char type[32];
    char name[64];
} Symbol;

Symbol table[MAX_SYMBOLS];
int symbolCount = 0;

// Trim leading spaces from a string
void trimLeading(char *str) {
    while (*str == ' ' || *str == '\t') str++;
}

// Add symbol to symbol table
void addSymbol(const char *type, const char *name) {
    strcpy(table[symbolCount].type, type);
    strcpy(table[symbolCount].name, name);
    symbolCount++;
}

// Display and store the symbol table
void displaySymbolTable(FILE *outFile) {
    printf("\nSymbol Table:\n");
    printf("---------------------------\n");
    printf("| %-10s | %-10s |\n", "Type", "Name");
    printf("---------------------------\n");

    fprintf(outFile, "Symbol Table:\n");
    fprintf(outFile, "---------------------------\n");
    fprintf(outFile, "| %-10s | %-10s |\n", "Type", "Name");
    fprintf(outFile, "---------------------------\n");

    for (int i = 0; i < symbolCount; i++) {
        printf("| %-10s | %-10s |\n", table[i].type, table[i].name);
        fprintf(outFile, "| %-10s | %-10s |\n", table[i].type, table[i].name);
    }

    printf("---------------------------\n");
    fprintf(outFile, "---------------------------\n");
}

// Parse the parse tree file and build the symbol table
void parseTree(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        printf("Error opening parse tree file.\n");
        exit(1);
    }

    char line[512];
    char lastType[32] = "";

    while (fgets(line, sizeof(line), fp)) {
        trimLeading(line);

        if (strstr(line, "ReturnType")) {
            sscanf(line, "%*[^()] (%[^)])", lastType); // Save the type for future identifiers
        }

        if (strstr(line, "Statement")) {
            sscanf(line, "%*[^()] (%[^)])", lastType); // Save the type for declared variables
        }

        if (strstr(line, "Identifier")) {
            char identifier[64];
            sscanf(line, "%*[^()] (%[^)])", identifier);

            // Avoid function names and printf/return
            if (strcmp(identifier, "main") != 0 &&
                strcmp(identifier, "printf") != 0 &&
                strcmp(identifier, "return") != 0) {

                // Check for duplicates
                int duplicate = 0;
                for (int i = 0; i < symbolCount; i++) {
                    if (strcmp(table[i].name, identifier) == 0) {
                        duplicate = 1;
                        break;
                    }
                }

                if (!duplicate) {
                    addSymbol(lastType, identifier);
                }
            }
        }
    }

    fclose(fp);
}

int main() {
    FILE *symbolFile = fopen("symbol_table.txt", "w");
    if (!symbolFile) {
        printf("Error opening symbol table file.\n");
        return 1;
    }

    parseTree("parse_tree.txt");
    displaySymbolTable(symbolFile);

    fclose(symbolFile);
    printf("Semantic analysis complete. Symbol table written to 'symbol_table.txt'\n");

    return 0;
}
