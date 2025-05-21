#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE 256
int tempVarCount = 0;

void trim(char *str) {
    int i = 0, j = 0;
    while (isspace(str[i])) i++;
    while (str[i]) str[j++] = str[i++];
    str[j] = '\0';
    for (i = strlen(str) - 1; i >= 0 && isspace(str[i]); i--)
        str[i] = '\0';
}

int isOperator(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/' || c == '>' || c == '<' || c == '=';
}

int precedence(char op) {
    switch (op) {
        case '*': case '/': return 3;
        case '+': case '-': return 2;
        case '>': case '<': case '=': return 1;
        default: return 0;
    }
}

typedef struct {
    char *items[128];
    int top;
} Stack;

void push(Stack *s, char *val) {
    s->items[++s->top] = strdup(val);
}

char *pop(Stack *s) {
    if (s->top == -1) return NULL;
    return s->items[s->top--];
}

char *peek(Stack *s) {
    if (s->top == -1) return NULL;
    return s->items[s->top];
}

void generateTACFromExpression(char *expr, FILE *out, char *targetVar) {
    // Tokenization
    char tokens[128][64];
    int tCount = 0;
    int i = 0;
    while (expr[i]) {
        if (isspace(expr[i])) {
            i++;
        } else if (isOperator(expr[i])) {
            tokens[tCount][0] = expr[i++];
            tokens[tCount++][1] = '\0';
        } else if (isalnum(expr[i])) {
            int j = 0;
            while (isalnum(expr[i]))
                tokens[tCount][j++] = expr[i++];
            tokens[tCount++][j] = '\0';
        } else {
            i++;
        }
    }

    // Shunting Yard to postfix
    Stack opStack = {.top = -1};
    char *postfix[128];
    int pCount = 0;

    for (int i = 0; i < tCount; i++) {
        char *token = tokens[i];
        if (isalnum(token[0])) {
            postfix[pCount++] = strdup(token);
        } else if (isOperator(token[0])) {
            while (opStack.top != -1 && precedence(peek(&opStack)[0]) >= precedence(token[0])) {
                postfix[pCount++] = pop(&opStack);
            }
            push(&opStack, token);
        }
    }
    while (opStack.top != -1)
        postfix[pCount++] = pop(&opStack);

    // TAC Generation from postfix
    Stack evalStack = {.top = -1};
    for (int i = 0; i < pCount; i++) {
        char *token = postfix[i];
        if (isalnum(token[0])) {
            push(&evalStack, token);
        } else {
            char *rhs = pop(&evalStack);
            char *lhs = pop(&evalStack);
            char temp[16];
            sprintf(temp, "t%d", tempVarCount++);
            fprintf(out, "%s = %s %s %s\n", temp, lhs, token, rhs);
            push(&evalStack, strdup(temp));
        }
    }

    char *result = pop(&evalStack);
    if (targetVar != NULL) {
        fprintf(out, "%s = %s\n", targetVar, result);
    }
}

int main() {
    FILE *in = fopen("input.c", "r");
    FILE *out = fopen("tac.txt", "w");
    if (!in || !out) {
        perror("File error");
        return 1;
    }

    printf("Generating TAC (Three Address Code):\n");

    char line[MAX_LINE];
    while (fgets(line, sizeof(line), in)) {
        trim(line);

        if (strstr(line, "int ") && strchr(line, '=')) {
            char var[32], expr[128];
            sscanf(line, "int %[^=]=%[^;];", var, expr);
            trim(var);
            trim(expr);
            generateTACFromExpression(expr, out, var);
            generateTACFromExpression(expr, stdout, var);
        } else if (strstr(line, "int ")) {
            char var[32];
            sscanf(line, "int %[^;];", var);
            trim(var);
            fprintf(out, "%s = 0\n", var);
            printf("%s = 0\n", var);
        } else if (strstr(line, "return ")) {
            char expr[128];
            sscanf(line, "return %[^;];", expr);
            trim(expr);
            generateTACFromExpression(expr, out, NULL);
            generateTACFromExpression(expr, stdout, NULL);
            fprintf(out, "return %s\n", expr);
            printf("return %s\n", expr);
        }
    }

    fclose(in);
    fclose(out);
    return 0;
}
