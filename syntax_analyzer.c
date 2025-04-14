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

int matchValue(char *expectedValue) {
    if (pos < total && strcmp(tokens[pos].value, expectedValue) == 0) {
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
    if (match("IDENTIFIER") || match("NUMBER")) return;
    printf("Syntax Error at token %d: expected IDENTIFIER or NUMBER, found <%s> %s\n",
           pos + 1, tokens[pos].type, tokens[pos].value);
    exit(1);
}

void expression() {
    factor();
    while (strcmp(tokens[pos].type, "OPERATOR") == 0) {
        pos++;
        factor();
    }
}

void return_statement() {
    expect("KEYWORD");  // return
    expression();
    expect("SEPARATOR"); // ;
}

void assignment() {
    expect("IDENTIFIER");
    expect("OPERATOR"); // =
    expression();
    expect("SEPARATOR"); // ;
}

void declaration() {
    expect("KEYWORD");       // int, float, etc.
    expect("IDENTIFIER");
    while (matchValue(",")) {
        expect("IDENTIFIER");
    }
    if (match("OPERATOR")) {
        expression();
    }
    expect("SEPARATOR");     // ;
}

void statement(); // forward declaration

void block() {
    expect("SEPARATOR"); // {
    while (strcmp(tokens[pos].value, "}") != 0 && pos < total) {
        statement();
    }
    expect("SEPARATOR"); // }
}

void parameter_list() {
    if (strcmp(tokens[pos].type, "KEYWORD") == 0) {
        expect("KEYWORD");
        expect("IDENTIFIER");
        while (matchValue(",")) {
            expect("KEYWORD");
            expect("IDENTIFIER");
        }
    }
}

void function_definition() {
    expect("KEYWORD");     // return type
    expect("IDENTIFIER");  // function name
    expect("SEPARATOR");   // (
    if (!matchValue(")")) {
        parameter_list();
        expect("SEPARATOR"); // )
    }
    block();
}

void function_call() {
    expect("IDENTIFIER");
    expect("SEPARATOR"); // (
    if (strcmp(tokens[pos].type, "IDENTIFIER") == 0 || strcmp(tokens[pos].type, "NUMBER") == 0) {
        expression();
        while (matchValue(",")) {
            expression();
        }
    }
    expect("SEPARATOR"); // )
    expect("SEPARATOR"); // ;
}

void if_statement() {
    expect("KEYWORD");  // if
    expect("SEPARATOR"); // (
    expression();
    expect("SEPARATOR"); // )
    block();
}

void statement() {
    if (strcmp(tokens[pos].type, "PREPROCESSOR") == 0) {
        pos++;
    } else if (strcmp(tokens[pos].type, "KEYWORD") == 0 &&
               strcmp(tokens[pos].value, "return") == 0) {
        return_statement();
    } else if (strcmp(tokens[pos].type, "KEYWORD") == 0 &&
               strcmp(tokens[pos].value, "if") == 0) {
        if_statement();
    } else if (strcmp(tokens[pos].type, "KEYWORD") == 0) {
        if (strcmp(tokens[pos + 1].type, "IDENTIFIER") == 0 &&
            strcmp(tokens[pos + 2].value, "(") == 0) {
            function_definition();
        } else {
            declaration();
        }
    } else if (strcmp(tokens[pos].type, "IDENTIFIER") == 0) {
        if (strcmp(tokens[pos + 1].value, "(") == 0) {
            function_call();
        } else {
            assignment();
        }
    } else if (strcmp(tokens[pos].value, "{") == 0) {
        block();
    } else {
        printf("Syntax Error at token %d: Unexpected token <%s> %s\n",
               pos + 1, tokens[pos].type, tokens[pos].value);
        exit(1);
    }
}

void parse() {
    while (pos < total) {
        statement();
    }
    printf("Syntax Analysis Successful \xE2\x9C\x85\n");
}

int main() {
    loadTokens();
    parse();
    return 0;
}
