#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX 100

typedef struct {
    char type[30];
    char value[100];
} Token;

Token tokens[MAX];
int pos = 0, total = 0;

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

// Match current token
int match(char *expectedType) {
    if (pos < total && strcmp(tokens[pos].type, expectedType) == 0) {
        pos++;
        return 1;
    }
    return 0;
}

// Expect and advance
void expect(char *expectedType) {
    if (!match(expectedType)) {
        printf("Syntax Error at token %d: expected %s, found %s (%s)\n", pos + 1, expectedType, tokens[pos].type, tokens[pos].value);
        exit(1);
    }
}

// Parser functions
void expression();
void statement();

void factor() {
    if (match("IDENTIFIER") || match("NUMBER")) {
        return;
    }
    printf("Syntax Error at token %d: invalid factor\n", pos + 1);
    exit(1);
}

void expression() {
    factor();
    while (match("OPERATOR")) {
        factor();
    }
}

void assignment() {
    expect("IDENTIFIER");
    expect("OPERATOR"); // expecting =
    expression();
    expect("SEPARATOR"); // expecting ;
}

void declaration() {
    expect("KEYWORD");       // int, float, etc.
    expect("IDENTIFIER");
    if (match("OPERATOR")) { // optional =
        expression();
    }
    expect("SEPARATOR");     // ;
}

void if_statement() {
    expect("KEYWORD");       // if
    expect("SEPARATOR");     // (
    expression();
    expect("SEPARATOR");     // )
    expect("SEPARATOR");     // {
    while (!match("SEPARATOR")) { // }
        statement();
    }
}

void statement() {
    if (strcmp(tokens[pos].type, "KEYWORD") == 0 && strcmp(tokens[pos].value, "if") == 0) {
        if_statement();
    }
    else if (strcmp(tokens[pos].type, "KEYWORD") == 0) {
        declaration();
    }
    else {
        assignment();
    }
}

void parse() {
    while (pos < total) {
        statement();
    }
    printf("Syntax Analysis Successful ✅\n");
}

int main() {
    loadTokens();
    parse();
    return 0;
}

