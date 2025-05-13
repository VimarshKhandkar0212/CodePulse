#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX 100

typedef struct {
    char type[30];
    char value[100];
} Token;

typedef struct Node {
    char type[50];
    char value[100];
    struct Node **children;
    int child_count;
    int max_children;
} Node;

Token tokens[MAX];
int pos = 0, total = 0;
Node *root = NULL;
FILE *output_file;

// Function declarations
void statement(Node *parent);
void parse();
void loadTokens();
void function_definition(Node *parent);
void if_statement(Node *parent);
void assignment_or_declaration(Node *parent);
void expression(Node *parent);
void factor(Node *parent);
void function_call(Node *parent);
Node *create_node(const char *type, const char *value);
void add_child(Node *parent, Node *child);
void print_tree(Node *node, int depth);
void free_tree(Node *node);

// Create a new parse tree node
Node *create_node(const char *type, const char *value) {
    Node *node = (Node *)malloc(sizeof(Node));
    strncpy(node->type, type, 49);
    node->type[49] = '\0';
    strncpy(node->value, value ? value : "", 99);
    node->value[99] = '\0';
    node->child_count = 0;
    node->max_children = 10;
    node->children = (Node **)malloc(node->max_children * sizeof(Node *));
    return node;
}

// Add a child to a parent node
void add_child(Node *parent, Node *child) {
    if (parent->child_count >= parent->max_children) {
        parent->max_children *= 2;
        parent->children = (Node **)realloc(parent->children, parent->max_children * sizeof(Node *));
    }
    parent->children[parent->child_count++] = child;
}

// Print the parse tree with indentation
void print_tree(Node *node, int depth) {
    if (!node) return;
    for (int i = 0; i < depth; i++) {
        printf("│   ");
        fprintf(output_file, "│   ");
    }
    printf("├── %s", node->type);
    fprintf(output_file, "├── %s", node->type);
    if (strlen(node->value) > 0) {
        printf(" (%s)", node->value);
        fprintf(output_file, " (%s)", node->value);
    }
    printf("\n");
    fprintf(output_file, "\n");
    for (int i = 0; i < node->child_count; i++) {
        print_tree(node->children[i], depth + 1);
    }
}

// Free the parse tree
void free_tree(Node *node) {
    if (!node) return;
    for (int i = 0; i < node->child_count; i++) {
        free_tree(node->children[i]);
    }
    free(node->children);
    free(node);
}

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

void expect(char *expectedType, Node *parent, const char *node_type) {
    if (!match(expectedType)) {
        printf("Syntax Error at token %d: expected %s, found <%s> %s\n",
               pos + 1, expectedType, tokens[pos].type, tokens[pos].value);
        exit(1);
    }
    if (node_type && parent) {
        Node *node = create_node(node_type, tokens[pos - 1].value);
        add_child(parent, node);
    }
}

void factor(Node *parent) {
    if (match("IDENTIFIER")) {
        Node *node = create_node("Identifier", tokens[pos - 1].value);
        add_child(parent, node);
    } else if (match("NUMBER")) {
        Node *node = create_node("Constant", tokens[pos - 1].value);
        add_child(parent, node);
    } else if (match("STRING_LITERAL")) {
        Node *node = create_node("StringLiteral", tokens[pos - 1].value);
        add_child(parent, node);
    } else if (match("CHAR_CONSTANT")) {
        Node *node = create_node("CharConstant", tokens[pos - 1].value);
        add_child(parent, node);
    } else {
        printf("Syntax Error at token %d: expected IDENTIFIER or NUMBER, found <%s> %s\n",
               pos + 1, tokens[pos].type, tokens[pos].value);
        exit(1);
    }
}

void expression(Node *parent) {
    Node *expr_node = create_node("Expression", "");
    add_child(parent, expr_node);
    factor(expr_node);
    while (match("OPERATOR")) {
        Node *op_node = create_node("Operator", tokens[pos - 1].value);
        add_child(expr_node, op_node);
        factor(expr_node);
    }
}

void assignment_or_declaration(Node *parent) {
    Node *decl_node = NULL;
    if (strcmp(tokens[pos].type, "KEYWORD") == 0) {
        decl_node = create_node("Declaration", "");
        add_child(parent, decl_node);
        Node *type_node = create_node("TypeSpecifier", tokens[pos].value);
        add_child(decl_node, type_node);
        pos++;
        expect("IDENTIFIER", decl_node, "Identifier");
        if (match("OPERATOR")) {
            Node *init_node = create_node("Initializer", "");
            add_child(decl_node, init_node);
            expression(init_node);
        }
        expect("SEPARATOR", decl_node, NULL); // ;
    } else {
        decl_node = create_node("Assignment", "");
        add_child(parent, decl_node);
        expect("IDENTIFIER", decl_node, "Identifier");
        expect("OPERATOR", decl_node, "Operator"); // =
        expression(decl_node);
        expect("SEPARATOR", decl_node, NULL); // ;
    }
}

void if_statement(Node *parent) {
    Node *if_node = create_node("IfStatement", "");
    add_child(parent, if_node);
    expect("KEYWORD", if_node, NULL); // if
    expect("SEPARATOR", if_node, NULL); // (
    expression(if_node);
    expect("SEPARATOR", if_node, NULL); // )
    expect("SEPARATOR", if_node, NULL); // {
    Node *compound_node = create_node("CompoundStatement", "");
    add_child(if_node, compound_node);
    while (pos < total && !(strcmp(tokens[pos].type, "SEPARATOR") == 0 && strcmp(tokens[pos].value, "}") == 0)) {
        statement(compound_node);
    }
    expect("SEPARATOR", if_node, NULL); // }
}

void function_definition(Node *parent) {
    Node *func_node = create_node("FunctionDefinition", "");
    add_child(parent, func_node);
    expect("KEYWORD", func_node, "TypeSpecifier"); // int
    expect("IDENTIFIER", func_node, "Declarator"); // function name
    expect("SEPARATOR", func_node, NULL); // (
    Node *param_list = create_node("ParameterList", "");
    add_child(func_node, param_list);
    if (!(strcmp(tokens[pos].type, "SEPARATOR") == 0 && strcmp(tokens[pos].value, ")") == 0)) {
        while (1) {
            Node *param = create_node("Parameter", "");
            add_child(param_list, param);
            expect("KEYWORD", param, "TypeSpecifier"); // type
            expect("IDENTIFIER", param, "Identifier"); // name
            if (strcmp(tokens[pos].type, "SEPARATOR") == 0 && strcmp(tokens[pos].value, ")") == 0) break;
            expect("SEPARATOR", param, NULL); // comma
        }
    }
    expect("SEPARATOR", func_node, NULL); // )
    expect("SEPARATOR", func_node, NULL); // {
    Node *compound_node = create_node("CompoundStatement", "");
    add_child(func_node, compound_node);
    while (pos < total && !(strcmp(tokens[pos].type, "SEPARATOR") == 0 && strcmp(tokens[pos].value, "}") == 0)) {
        statement(compound_node);
    }
    expect("SEPARATOR", func_node, NULL); // }
}

void function_call(Node *parent) {
    Node *call_node = create_node("FunctionCall", "");
    add_child(parent, call_node);
    expect("IDENTIFIER", call_node, "Identifier"); // function name
    expect("SEPARATOR", call_node, NULL); // (
    Node *arg_list = create_node("ArgumentList", "");
    add_child(call_node, arg_list);
    if (!(strcmp(tokens[pos].type, "SEPARATOR") == 0 && strcmp(tokens[pos].value, ")") == 0)) {
        expression(arg_list);
        while (strcmp(tokens[pos].type, "SEPARATOR") == 0 && strcmp(tokens[pos].value, ",") == 0) {
            pos++;
            expression(arg_list);
        }
    }
    expect("SEPARATOR", call_node, NULL); // )
    expect("SEPARATOR", call_node, NULL); // ;
}

void statement(Node *parent) {
    if (strcmp(tokens[pos].type, "PREPROCESSOR") == 0) {
        Node *prep_node = create_node("PreprocessorDirective", tokens[pos].value);
        add_child(parent, prep_node);
        pos++;
        return;
    }

    if (strcmp(tokens[pos].type, "KEYWORD") == 0 &&
        strcmp(tokens[pos].value, "if") == 0) {
        if_statement(parent);
    }
    else if (strcmp(tokens[pos].type, "KEYWORD") == 0 &&
             strcmp(tokens[pos + 1].type, "IDENTIFIER") == 0 &&
             strcmp(tokens[pos + 2].type, "SEPARATOR") == 0 &&
             strcmp(tokens[pos + 2].value, "(") == 0) {
        function_definition(parent);
    }
    else if (strcmp(tokens[pos].type, "IDENTIFIER") == 0 &&
             strcmp(tokens[pos + 1].type, "SEPARATOR") == 0 &&
             strcmp(tokens[pos + 1].value, "(") == 0) {
        Node *expr_stmt = create_node("ExpressionStatement", "");
        add_child(parent, expr_stmt);
        function_call(expr_stmt);
    }
    else if (strcmp(tokens[pos].type, "KEYWORD") == 0 &&
             strcmp(tokens[pos].value, "return") == 0) {
        Node *return_node = create_node("ReturnStatement", "");
        add_child(parent, return_node);
        pos++;
        if (strcmp(tokens[pos].type, "SEPARATOR") != 0 ||
            strcmp(tokens[pos].value, ";") != 0) {
            expression(return_node);
        }
        expect("SEPARATOR", return_node, NULL); // ;
    }
    else {
        assignment_or_declaration(parent);
    }
}

void parse() {
    root = create_node("TranslationUnit", "");
    while (pos < total) {
        statement(root);
    }
    printf("Syntax Analysis Successful ✅\n");
    printf("\nParse Tree:\n");
    output_file = fopen("parse_tree.txt", "w");
    if (!output_file) {
        printf("Could not open parse_tree.txt for writing\n");
        exit(1);
    }
    print_tree(root, 0);
    fclose(output_file);
}

int main() {
    loadTokens();
    parse();
    free_tree(root);
    return 0;
}
