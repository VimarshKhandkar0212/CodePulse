#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 256

// Structure for a symbol
typedef struct {
    char name[50];
    char type[50];
    char scope[50];
} Symbol;

Symbol symbolTable[100];
int symbolCount = 0;

void addSymbol(const char* name, const char* type, const char* scope) {
    strcpy(symbolTable[symbolCount].name, name);
    strcpy(symbolTable[symbolCount].type, type);
    strcpy(symbolTable[symbolCount].scope, scope);
    symbolCount++;
}

void printSymbolTable() {
    FILE *fp = fopen("symbol_table.txt", "w");
    if (fp == NULL) {
        printf("❌ Error opening symbol_table.txt for writing.\n");
        return;
    }

    printf("\n📄 Symbol Table:\n");
    printf("--------------------------------------------------\n");
    printf("%-20s %-15s %-15s\n", "Name", "Type", "Scope");
    printf("--------------------------------------------------\n");

    fprintf(fp, "Symbol Table:\n");
    fprintf(fp, "--------------------------------------------------\n");
    fprintf(fp, "%-20s %-15s %-15s\n", "Name", "Type", "Scope");
    fprintf(fp, "--------------------------------------------------\n");

    for (int i = 0; i < symbolCount; i++) {
        printf("%-20s %-15s %-15s\n",
               symbolTable[i].name,
               symbolTable[i].type,
               symbolTable[i].scope);

        fprintf(fp, "%-20s %-15s %-15s\n",
                symbolTable[i].name,
                symbolTable[i].type,
                symbolTable[i].scope);
    }

    printf("--------------------------------------------------\n");
    fprintf(fp, "--------------------------------------------------\n");

    printf("\n✅ Symbol table created and saved to 'symbol_table.txt'.\n");
    fprintf(fp, "\nSymbol table saved successfully to 'symbol_table.txt'.\n");

    fclose(fp);
}

void parseLine(char* line, char* currentScope) {
    if (strstr(line, "+-- #include")) {
    char header[60];
    if (sscanf(line, "|   |   +-- #include<%[^>]>", header) == 1) {
        char headerWithBrackets[70];
        sprintf(headerWithBrackets, "<%s>", header);
        addSymbol(headerWithBrackets, "Header File", "Global");
    }
}

    else if (strstr(line, "Return Type:")) {
        char returnType[50];
        sscanf(line, "|   |   +-- Return Type: %s", returnType);
        strcpy(currentScope, "main");
        addSymbol("main", returnType, "Global");
    }

    else if (strstr(line, "Initialization:")) {
        char var[50];
        sscanf(line, "|   |   |   |   +-- Initialization: %[^= ]", var);
        addSymbol(var, "int", currentScope);
    }

    else if (strstr(line, "Constant:")) {
        char constVal[50];
        sscanf(line, "|   |   |   |   +-- Constant: %s", constVal);
        addSymbol(constVal, "const int", currentScope);
    }
}

int main() {
    FILE *fp = fopen("parse_tree.txt", "r");
    if (fp == NULL) {
        printf("❌ Failed to open parse_tree.txt.\n");
        return 1;
    }

    char line[MAX_LINE_LENGTH];
    char currentScope[50] = "Global";

    while (fgets(line, sizeof(line), fp)) {
        parseLine(line, currentScope);
    }

    fclose(fp);
    printSymbolTable();
    return 0;
}
