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

// Function declarations
void statement();
void parse();
void loadTokens();
void function_definition();
void if_statement();
void assignment_or_declaration();
void expression();
void factor();

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

int match(char *expectedType) {
    if (pos < total && strcmp(tokens[pos].type, expectedType) == 0) {
        pos++;
        return 1;
    }
    return 0;
}

void expect(char *expectedType) {
    if (!match(expectedType)) {
        printf("Syntax Error at token %d: expected %s, found <%s> %s\n",
               pos + 1, expectedType, tokens[pos].type, tokens[pos].value);
        exit(1);
    }
}

void factor() {
    if (match("IDENTIFIER") || match("NUMBER") || match("STRING_LITERAL") || match("CHAR_CONSTANT")) {
        return;
    }
    printf("Syntax Error at token %d: expected IDENTIFIER or NUMBER, found <%s> %s\n",
           pos + 1, tokens[pos].type, tokens[pos].value);
    exit(1);
}

void expression() {
    factor();
    while (match("OPERATOR")) {
        factor();
    }
}

void assignment_or_declaration() {
    if (strcmp(tokens[pos].type, "KEYWORD") == 0) {
        pos++; // keyword (int, float, char...)
        expect("IDENTIFIER");
        if (match("OPERATOR")) {
            expression();
        }
        expect("SEPARATOR"); // ;
    } else {
        expect("IDENTIFIER");
        expect("OPERATOR"); // =
        expression();
        expect("SEPARATOR"); // ;
    }
}

void if_statement() {
    expect("KEYWORD"); // if
    expect("SEPARATOR"); // (
    expression();
    expect("SEPARATOR"); // )
    expect("SEPARATOR"); // {
    while (pos < total && !(strcmp(tokens[pos].type, "SEPARATOR") == 0 && strcmp(tokens[pos].value, "}") == 0)) {
        statement();
    }
    expect("SEPARATOR"); // }
}

void function_definition() {
    expect("KEYWORD");    // int
    expect("IDENTIFIER"); // function name
    expect("SEPARATOR");  // (
    // Parse parameters
    if (!(strcmp(tokens[pos].type, "SEPARATOR") == 0 && strcmp(tokens[pos].value, ")") == 0)) {
        while (1) {
            expect("KEYWORD");      // type
            expect("IDENTIFIER");   // name
            if (strcmp(tokens[pos].type, "SEPARATOR") == 0 && strcmp(tokens[pos].value, ")") == 0) break;
            expect("SEPARATOR");    // comma
        }
    }
    expect("SEPARATOR"); // )
    expect("SEPARATOR"); // {
    while (pos < total && !(strcmp(tokens[pos].type, "SEPARATOR") == 0 && strcmp(tokens[pos].value, "}") == 0)) {
        statement();
    }
    expect("SEPARATOR"); // }
}

void statement() {
    if (strcmp(tokens[pos].type, "PREPROCESSOR") == 0) {
        pos++;
        return;
    }

    if (strcmp(tokens[pos].type, "KEYWORD") == 0 &&
        strcmp(tokens[pos].value, "if") == 0) {
        if_statement();
    }
    else if (strcmp(tokens[pos].type, "KEYWORD") == 0 &&
             strcmp(tokens[pos + 1].type, "IDENTIFIER") == 0 &&
             strcmp(tokens[pos + 2].type, "SEPARATOR") == 0 &&
             strcmp(tokens[pos + 2].value, "(") == 0) {
        function_definition();
    }
    else if (strcmp(tokens[pos].type, "KEYWORD") == 0 &&
             strcmp(tokens[pos].value, "return") == 0) {
        pos++;
        if (strcmp(tokens[pos].type, "SEPARATOR") != 0 ||
            strcmp(tokens[pos].value, ";") != 0) {
            expression();
        }
        expect("SEPARATOR"); // ;
    }
    else {
        assignment_or_declaration();
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
