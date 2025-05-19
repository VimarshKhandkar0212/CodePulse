#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_LEVEL 100

FILE *fp;
FILE *output;

char type[50], token[100];
int hasSibling[MAX_LEVEL];
bool parseError = false;

void reportError(const char *msg) {
    fprintf(stderr, "Syntax Error: %s\n", msg);
    parseError = true;
}

void printPrefix(int level) {
    for (int i = 0; i < level - 1; i++) {
        if (hasSibling[i]) printf("|   "), fprintf(output, "|   ");
        else printf("    "), fprintf(output, "    ");
    }
    if (level > 0) printf("+-- "), fprintf(output, "+-- ");
}

void printNode(int level, int sibling, const char *label, const char *content) {
    hasSibling[level] = sibling;
    printPrefix(level);
    if (content) printf("%s: %s\n", label, content), fprintf(output, "%s: %s\n", label, content);
    else printf("%s\n", label), fprintf(output, "%s\n", label);
}

int isInclude(const char *token) {
    return strstr(token, "#include") != NULL;
}

int isDatatype(const char *token) {
    return strcmp(token, "int") == 0 || strcmp(token, "float") == 0 || strcmp(token, "char") == 0 || strcmp(token, "void") == 0;
}

int isIdentifier(const char *type) {
    return strcmp(type, "IDENTIFIER") == 0;
}

int isNumber(const char *type) {
    return strcmp(type, "NUMBER") == 0;
}

// Forward declarations
void parseTranslationUnit();
void parseFunction(int);
void parseParameters(int);
void parseFunctionBody(int);
void parseDeclaration(int);
void parseForLoop(int);
void parseReturnStatement(int);

void parseTranslationUnit() {
    printNode(0, 0, "TranslationUnit", NULL);

    long fileStart = ftell(fp);
    int includesCount = 0;
    while (fscanf(fp, "%s %s", type, token) != EOF) {
        if (strcmp(type, "PREPROCESSOR") == 0 && isInclude(token)) includesCount++;
    }
    fseek(fp, fileStart, SEEK_SET);

    int count = includesCount;
    while (count--) {
        if (fscanf(fp, "%s %s", type, token) == EOF) break;
        if (strcmp(type, "PREPROCESSOR") == 0 && isInclude(token)) {
            printNode(1, (count > 0), "Include Directives", NULL);
            printNode(2, 0, token, NULL);
        }
    }

    while (!parseError && fscanf(fp, "%s %s", type, token) != EOF) {
        if (strcmp(type, "KEYWORD") == 0) {
            if (isDatatype(token) || strcmp(token, "void") == 0) {
                long pos = ftell(fp);
                char nextType[50], nextToken[100];
                if (fscanf(fp, "%s %s", nextType, nextToken) != EOF) {
                    if (isIdentifier(nextType)) {
                        long pos2 = ftell(fp);
                        char tType[50], tToken[100];
                        if (fscanf(fp, "%s %s", tType, tToken) != EOF) {
                            if (strcmp(tToken, "(") == 0) {
                                fseek(fp, pos, SEEK_SET);
                                parseFunction(1);
                            } else {
                                fseek(fp, pos, SEEK_SET);
                                parseDeclaration(1);
                            }
                        } else fseek(fp, pos, SEEK_SET);
                    } else fseek(fp, pos, SEEK_SET);
                } else fseek(fp, pos, SEEK_SET);
            } else if (strcmp(token, "for") == 0) {
                parseForLoop(1);
            } else if (strcmp(token, "return") == 0) {
                parseReturnStatement(1);
            }
        }
    }
}

void parseFunction(int level) {
    if (parseError) return;
    printNode(level, 0, "Function Definition", NULL);
    printNode(level + 1, 1, "Return Type", token);

    if (fscanf(fp, "%s %s", type, token) == EOF) return reportError("Unexpected EOF after return type");
    if (isIdentifier(type)) printNode(level + 1, 1, "Function Name", token);
    else return reportError("Expected function name");

    if (fscanf(fp, "%s %s", type, token) == EOF || strcmp(token, "(") != 0)
        return reportError("Expected '(' after function name");

    printNode(level + 1, 1, "Parameters", NULL);
    parseParameters(level + 2);

    if (fscanf(fp, "%s %s", type, token) == EOF || strcmp(token, ")") != 0)
        return reportError("Expected ')' after parameters");

    if (fscanf(fp, "%s %s", type, token) == EOF || strcmp(token, "{") != 0)
        return reportError("Expected '{' to start function body");

    printNode(level + 1, 0, "Body (Compound Statement)", NULL);
    parseFunctionBody(level + 2);
}

void parseParameters(int level) {
    if (parseError) return;
    long pos;
    while (true) {
        pos = ftell(fp);
        if (fscanf(fp, "%s %s", type, token) == EOF) return reportError("Unexpected end in parameters");
        if (strcmp(token, ")") == 0) { fseek(fp, pos, SEEK_SET); break; }
        if (isDatatype(token)) {
            printNode(level, 0, "Parameter", NULL);
            printNode(level + 1, 1, "Type", token);
            if (fscanf(fp, "%s %s", type, token) == EOF || !isIdentifier(type))
                return reportError("Expected identifier in parameter");
            printNode(level + 1, 0, "Identifier", token);

            pos = ftell(fp);
            if (fscanf(fp, "%s %s", type, token) == EOF) return;
            if (strcmp(token, ",") == 0) continue;
            else if (strcmp(token, ")") == 0) { fseek(fp, pos, SEEK_SET); break; }
            else return reportError("Expected ',' or ')'");
        } else return reportError("Expected type in parameters");
    }
}

void parseFunctionBody(int level) {
    if (parseError) return;
    while (true) {
        long pos = ftell(fp);
        if (fscanf(fp, "%s %s", type, token) == EOF) return reportError("Unexpected EOF in body");
        if (strcmp(token, "}") == 0) break;
        else if (strcmp(type, "KEYWORD") == 0) {
            if (isDatatype(token)) parseDeclaration(level);
            else if (strcmp(token, "for") == 0) parseForLoop(level);
            else if (strcmp(token, "return") == 0) parseReturnStatement(level);
            else return reportError("Unexpected keyword in body");
        }
    }
}

void parseDeclaration(int level) {
    if (parseError) return;
    printNode(level, 0, "Declaration", NULL);
    printNode(level + 1, 1, "Type", token);
    if (fscanf(fp, "%s %s", type, token) == EOF || !isIdentifier(type))
        return reportError("Expected identifier");
    printNode(level + 1, 0, "Identifier", token);

    if (fscanf(fp, "%s %s", type, token) == EOF) return;
    if (strcmp(token, "=") == 0) {
        if (fscanf(fp, "%s %s", type, token) == EOF)
            return reportError("Expected initializer");
        printNode(level + 1, 1, "Initializer", token);
        if (fscanf(fp, "%s %s", type, token) == EOF || strcmp(token, ";") != 0)
            return reportError("Expected ';' after initializer");
    } else if (strcmp(token, ";") != 0) {
        return reportError("Expected ';' or '='");
    }
}

void parseForLoop(int level) {
    if (parseError) return;
    printNode(level, 0, "For Loop", NULL);
    if (fscanf(fp, "%s %s", type, token) == EOF || strcmp(token, "(") != 0)
        return reportError("Expected '(' after for");

    printNode(level + 1, 1, "Initialization", NULL);
    if (fscanf(fp, "%s %s", type, token) == EOF) return;
    if (isDatatype(token)) parseDeclaration(level + 2);
    else return reportError("Expected declaration in for init");

    printNode(level + 1, 1, "Condition", NULL);
    char expr[256] = "";
    while (fscanf(fp, "%s %s", type, token) != EOF && strcmp(token, ";") != 0)
        strcat(expr, token), strcat(expr, " ");
    printNode(level + 2, 0, "Expression", expr);

    printNode(level + 1, 1, "Increment", NULL);
    expr[0] = 0;
    while (fscanf(fp, "%s %s", type, token) != EOF && strcmp(token, ")") != 0)
        strcat(expr, token), strcat(expr, " ");
    printNode(level + 2, 0, "Expression", expr);

    if (fscanf(fp, "%s %s", type, token) == EOF || strcmp(token, "{") != 0)
        return reportError("Expected '{' after for loop");
    printNode(level + 1, 0, "Body (Compound Statement)", NULL);
    parseFunctionBody(level + 2);
}

void parseReturnStatement(int level) {
    if (parseError) return;
    printNode(level, 0, "Return Statement", NULL);
    char expr[256] = "";
    while (fscanf(fp, "%s %s", type, token) != EOF && strcmp(token, ";") != 0)
        strcat(expr, token), strcat(expr, " ");
    printNode(level + 1, 0, "Expression", expr);
}

int main() {
    fp = fopen("tokens.txt", "r");
    if (!fp) {
        perror("Could not open input.txt");
        return 1;
    }
    output = fopen("parse_tree.txt", "w");
    if (!output) {
        perror("Could not open output.txt");
        fclose(fp);
        return 1;
    }

    parseTranslationUnit();

    if (parseError) {
        printf("Parsing terminated due to syntax errors.\n");
    } else {
        printf("Parsing completed successfully.\n");
    }

    fclose(fp);
    fclose(output);
    return 0;
}
