#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

FILE *fp, *tac;
char type[50], token[100];
int tempCount = 1;

typedef struct {
    char token[50];
    char type[50];
} ExprToken;

char buffer[200];  // Add this as a global or inside the function

bool nextToken() {
    if (fgets(buffer, sizeof(buffer), fp) == NULL)
        return false;

    // This reads entire second field (token), including spaces, until newline
    return sscanf(buffer, "%s %[^\n]", type, token) == 2;
}


int precedence(char *op) {
    if (strcmp(op, "+") == 0 || strcmp(op, "-") == 0) return 1;
    if (strcmp(op, "*") == 0 || strcmp(op, "/") == 0) return 2;
    return 0;
}

bool isLeftParen(char *t) {
    return strcmp(t, "(") == 0;
}

bool isRightParen(char *t) {
    return strcmp(t, ")") == 0;
}

bool isOperator(char *op) {
    return strcmp(op, "+") == 0 || strcmp(op, "-") == 0 ||
           strcmp(op, "*") == 0 || strcmp(op, "/") == 0 ||
           strcmp(op, ">") == 0 || strcmp(op, "<") == 0 ||
           strcmp(op, ">=") == 0 || strcmp(op, "<=") == 0 ||
           strcmp(op, "==") == 0 || strcmp(op, "!=") == 0 ||
           strcmp(op, "=") == 0;
}

void processExpression(char resultVar[]);

void handleIOCall() {
    char funcName[50];
    strcpy(funcName, token); // "printf" or "scanf"

    if (!nextToken() || !(strcmp(type, "SEPARATOR") == 0 && strcmp(token, "(") == 0)) {
        printf("Error: expected '(' after %s\n", funcName);
        return;
    }

    int paramCount = 0;

    while (nextToken() && !(strcmp(type, "SEPARATOR") == 0 && strcmp(token, ")") == 0)) {
        if (strcmp(type, "SEPARATOR") == 0 && strcmp(token, ",") == 0)
            continue;

        if (strcmp(type, "OPERATOR") == 0 && strcmp(token, "&") == 0) {
            if (!nextToken()) continue;

if (strcmp(type, "IDENTIFIER") != 0) {
    printf("Error: expected IDENTIFIER, got %s\n", token);
    continue;
}

char maybeFuncName[50];
strcpy(maybeFuncName, token);

if (!nextToken()) continue;

if (strcmp(type, "SEPARATOR") == 0 && strcmp(token, "(") == 0) {
    // Likely function → rewind and let function parser handle it
    fseek(fp, -strlen(buffer), SEEK_CUR); // undo reading '('
    fseek(fp, -strlen(maybeFuncName) - 5, SEEK_CUR); // rewind IDENTIFIER line
    continue;
}
            fprintf(tac, "param %s\n", token);
            paramCount++;
        } else if (strcmp(type, "STRING_LITERAL") == 0 ||
                   strcmp(type, "IDENTIFIER") == 0 ||
                   strcmp(type, "NUMBER") == 0) {
            fprintf(tac, "param %s\n", token);
            paramCount++;
        } else {
            printf("Warning: unexpected token '%s' in %s call\n", token, funcName);
        }
    }

    nextToken(); // should be ;
    fprintf(tac, "call %s, %d\n", funcName, paramCount);
}

void generateTAC() {
    while (nextToken()) {
        // 1. Handle printf / scanf
        if (strcmp(type, "IDENTIFIER") == 0 &&
            (strcmp(token, "printf") == 0 || strcmp(token, "scanf") == 0)) {
            handleIOCall();
            continue;
        }

        // 2. Handle return statements
        if (strcmp(type, "KEYWORD") == 0 && strcmp(token, "return") == 0) {
            if (!nextToken()) continue;
            char result[50] = "__ret";
            processExpression(result);
            fprintf(tac, "return %s\n", result);
            continue;
        }

        // 3. Handle function definitions
        if (strcmp(type, "KEYWORD") == 0 &&
            (strcmp(token, "int") == 0 || strcmp(token, "float") == 0 ||
             strcmp(token, "void") == 0 || strcmp(token, "char") == 0)) {

            char returnType[50];
            strcpy(returnType, token);

            if (!nextToken()) continue;

            if (strcmp(type, "IDENTIFIER") == 0) {
                char funcName[50];
                strcpy(funcName, token);

                if (!nextToken()) continue;

                if (strcmp(type, "SEPARATOR") == 0 && strcmp(token, "(") == 0) {
                    // Function detected
                    fprintf(tac, "func %s:\n", funcName);

                    // Properly parse parameter list
                    while (nextToken()) {
                        if (strcmp(type, "SEPARATOR") == 0 && strcmp(token, ")") == 0) break;

                        if (strcmp(type, "SEPARATOR") == 0 && strcmp(token, ",") == 0) continue;

                        if (strcmp(type, "KEYWORD") == 0) {
                            char paramType[50];
                            strcpy(paramType, token);

                            if (!nextToken() || strcmp(type, "IDENTIFIER") != 0) {
                                printf("Error: expected identifier after parameter type %s\n", paramType);
                                break;
                            }

                            char paramName[50];
                            strcpy(paramName, token);

                            // Emit TAC to declare param (optional comment)
                            fprintf(tac, "%s = param\n", paramName);
                        }
                    }

                    // Skip optional opening brace {
                    nextToken();

                    // Parse function body until }
                    while (nextToken() && !(strcmp(type, "SEPARATOR") == 0 && strcmp(token, "}") == 0)) {

                        if (strcmp(type, "IDENTIFIER") == 0 &&
                            (strcmp(token, "printf") == 0 || strcmp(token, "scanf") == 0)) {
                            handleIOCall();
                            continue;
                        }

                        if (strcmp(type, "KEYWORD") == 0 && strcmp(token, "return") == 0) {
                            if (!nextToken()) continue;
                            char result[50] = "__ret";
                            processExpression(result);
                            fprintf(tac, "return %s\n", result);
                            continue;
                        }

                        if (strcmp(type, "KEYWORD") == 0) {
                            char datatype[50];
                            strcpy(datatype, token);

                            if (!nextToken() || strcmp(type, "IDENTIFIER") != 0) continue;
                            char varname[50];
                            strcpy(varname, token);

                            if (!nextToken()) continue;

                            if (strcmp(token, "=") == 0) {
                                if (!nextToken()) continue;
                                processExpression(varname);
                            } else if (strcmp(token, ";") == 0) {
                                continue;
                            }
                        }
                    }

                    fprintf(tac, "endfunc\n");
                    continue;
                }
            }
        }

        // 4. Global variable declarations / assignments
// 4. Global variable declarations / assignments
// 4. Global variable declarations / assignments
if (strcmp(type, "KEYWORD") == 0) {
    char datatype[50];
    strcpy(datatype, token);

    // Process variables until semicolon
    while (true) {
        if (!nextToken()) {
            printf("Error: unexpected end of input\n");
            break;
        }
        if (strcmp(type, "SEPARATOR") == 0 && strcmp(token, ";") == 0) {
            break; // End of declaration
        }
        if (strcmp(type, "IDENTIFIER") != 0) {
            printf("Error: expected IDENTIFIER, got %s\n", token);
            continue;
        }
        char varname[50];
        strcpy(varname, token);

        if (!nextToken()) {
            printf("Error: unexpected end of input after %s\n", varname);
            break;
        }

        // Handle assignment
        if (strcmp(type, "OPERATOR") == 0 && strcmp(token, "=") == 0) {
            if (!nextToken()) {
                printf("Error: unexpected end of input after '='\n");
                break;
            }
            processExpression(varname);
        } else {
            // No assignment, rewind to reprocess this token as a potential separator
            fseek(fp, -strlen(buffer), SEEK_CUR);
        }

        // Check current token (set by processExpression or rewound above)
        if (!nextToken()) {
            printf("Error: unexpected end of input after expression\n");
            break;
        }
        if (strcmp(type, "SEPARATOR") == 0 && strcmp(token, ",") == 0) {
            continue; // Process next variable
        } else if (strcmp(type, "SEPARATOR") == 0 && strcmp(token, ";") == 0) {
            break; // End of declaration
        } else {
            printf("Error: expected ',' or ';', got %s\n", token);
            break;
        }
    }
    continue;
}
    }
}

void processExpression(char resultVar[]) {
    ExprToken expr[100];
    char postfix[100][50];
    char stack[100][50];
    int exprCount = 0, postCount = 0, stackTop = -1;

    // Add first token
    strcpy(expr[exprCount].token, token);
    strcpy(expr[exprCount++].type, type);

    // Collect tokens until , or ;
    while (nextToken()) {
        if (strcmp(type, "SEPARATOR") == 0 && (strcmp(token, ";") == 0 || strcmp(token, ",") == 0)) {
            // Rewind to leave , or ; for the caller
            fseek(fp, -strlen(buffer), SEEK_CUR);
            break;
        }
        strcpy(expr[exprCount].token, token);
        strcpy(expr[exprCount++].type, type);
    }

    // Tag unary operators
    for (int i = 0; i < exprCount; i++) {
        if (strcmp(expr[i].type, "OPERATOR") == 0 &&
            (i == 0 || strcmp(expr[i - 1].token, "(") == 0 || strcmp(expr[i - 1].token, "=") == 0)) {
            strcpy(expr[i].type, "UNARY");
            if (strcmp(expr[i].token, "-") == 0)
                strcpy(expr[i].token, "u-");
            else if (strcmp(expr[i].token, "+") == 0)
                strcpy(expr[i].token, "u+");
        }
    }

    // Infix to Postfix conversion (Shunting Yard)
    for (int i = 0; i < exprCount; i++) {
        if (strcmp(expr[i].type, "IDENTIFIER") == 0 || strcmp(expr[i].type, "NUMBER") == 0) {
            strcpy(postfix[postCount++], expr[i].token);
        } else if (strcmp(expr[i].type, "OPERATOR") == 0) {
            while (stackTop >= 0 && !isLeftParen(stack[stackTop]) &&
                   precedence(stack[stackTop]) >= precedence(expr[i].token)) {
                strcpy(postfix[postCount++], stack[stackTop--]);
            }
            strcpy(stack[++stackTop], expr[i].token);
        } else if (strcmp(expr[i].type, "UNARY") == 0) {
            strcpy(stack[++stackTop], expr[i].token);
        } else if (strcmp(expr[i].type, "SEPARATOR") == 0 && isLeftParen(expr[i].token)) {
            strcpy(stack[++stackTop], expr[i].token);
        } else if (strcmp(expr[i].type, "SEPARATOR") == 0 && isRightParen(expr[i].token)) {
            while (stackTop >= 0 && !isLeftParen(stack[stackTop])) {
                strcpy(postfix[postCount++], stack[stackTop--]);
            }
            if (stackTop >= 0 && isLeftParen(stack[stackTop])) {
                stackTop--;  // pop '('
            }
        }
    }

    while (stackTop >= 0) {
        strcpy(postfix[postCount++], stack[stackTop--]);
    }

    // TAC Generation from postfix
    char tempVars[100][50];
    int tempTop = -1;

    for (int i = 0; i < postCount; i++) {
        if (isOperator(postfix[i])) {
            char op2[50], op1[50], temp[10];
            strcpy(op2, tempVars[tempTop--]);
            strcpy(op1, tempVars[tempTop--]);
            sprintf(temp, "t%d", tempCount++);
            fprintf(tac, "%s = %s %s %s\n", temp, op1, postfix[i], op2);
            strcpy(tempVars[++tempTop], temp);
        } else if (strcmp(postfix[i], "u-") == 0 || strcmp(postfix[i], "u+") == 0) {
            char op[50], temp[10];
            strcpy(op, tempVars[tempTop--]);
            sprintf(temp, "t%d", tempCount++);
            fprintf(tac, "%s = %c%s\n", temp, postfix[i][1], op); // postfix[i][1] gives '-' or '+'
            strcpy(tempVars[++tempTop], temp);
        } else {
            strcpy(tempVars[++tempTop], postfix[i]);
        }
    }

    fprintf(tac, "%s = %s\n", resultVar, tempVars[tempTop]);
}

int main() {
    fp = fopen("tokens2.txt", "r");
    tac = fopen("tac.txt", "w");

    if (!fp || !tac) {
        perror("Error opening file");
        return 1;
    }

    generateTAC();

    fclose(fp);
    fclose(tac);

    printf("TAC generated in tac.txt\n");
    return 0;
}
