#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SYMBOLS 100
#define MAX_LINE 1024

typedef struct {
    char name[100];
    char type[50];
    char scope[20]; // "global" or function name
} Symbol;

Symbol table[MAX_SYMBOLS];
int symbolCount = 0;

// Check if symbol is already declared in the same scope
int isDuplicate(const char* name, const char* scope) {
    for (int i = 0; i < symbolCount; i++) {
        if (strcmp(table[i].name, name) == 0 && strcmp(table[i].scope, scope) == 0)
            return 1;
    }
    return 0;
}

void addSymbol(const char* name, const char* type, const char* scope) {
    if (isDuplicate(name, scope)) {
        printf("Semantic Error: Duplicate declaration of '%s' in scope '%s'\n", name, scope);
        return;
    }

    strcpy(table[symbolCount].name, name);
    strcpy(table[symbolCount].type, type);
    strcpy(table[symbolCount].scope, scope);
    symbolCount++;
}

void printSymbolTable(FILE* fout) {
    printf("\nSymbol Table:\n");
    printf("%-20s %-10s %-10s\n", "Name", "Type", "Scope");
    fprintf(fout, "\nSymbol Table:\n");
    fprintf(fout, "%-20s %-10s %-10s\n", "Name", "Type", "Scope");

    for (int i = 0; i < symbolCount; i++) {
        printf("%-20s %-10s %-10s\n", table[i].name, table[i].type, table[i].scope);
        fprintf(fout, "%-20s %-10s %-10s\n", table[i].name, table[i].type, table[i].scope);
    }
}

int main() {
    FILE* fin = fopen("parse_tree.txt", "r");
    if (!fin) {
        printf("Error: Could not open output.txt\n");
        return 1;
    }

    FILE* fout = fopen("symbol_table.txt", "w");
    if (!fout) {
        printf("Error: Could not open symbol_table.txt\n");
        fclose(fin);
        return 1;
    }

    char line[MAX_LINE];
    char currentScope[100] = "global";
    char lastType[50] = "";

    while (fgets(line, sizeof(line), fin)) {
        if (strstr(line, "Function Definition")) {
            // Nothing to extract directly, wait for name and return type
        } else if (strstr(line, "Return Type:")) {
            sscanf(line, "%*[^:] : %s", lastType);
        } else if (strstr(line, "Function Name:")) {
            char name[100];
            sscanf(line, "%*[^:] : %s", name);
            addSymbol(name, lastType, "global");
            strcpy(currentScope, name); // switch scope to function
        } else if (strstr(line, "Parameter")) {
            // Wait for next Type and Identifier
        } else if (strstr(line, "Type:")) {
            sscanf(line, "%*[^:] : %s", lastType);
        } else if (strstr(line, "Identifier:")) {
            char name[100];
            sscanf(line, "%*[^:] : %s", name);
            addSymbol(name, lastType, currentScope);
        } else if (strstr(line, "Declaration")) {
            // Wait for next Type and Identifier
        } else if (strstr(line, "Body (Compound Statement)")) {
            // Scope can remain the same
        }
    }

    printSymbolTable(fout);
    fclose(fin);
    fclose(fout);
    return 0;
}
