#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

typedef enum {
    EXPR_BINARY,
    EXPR_IDENTIFIER,
    EXPR_NUMBER,
    EXPR_STRING
} ExprType;

typedef struct Expr {
    ExprType type;
    struct Expr *left;
    struct Expr *right;
    Token op;
    char *name;
    int value;
    char *stringVal;
} Expr;

typedef enum {
    STMT_ASSIGN,
    STMT_PRINT,
    STMT_IF,
    STMT_WHILE,
    STMT_BLOCK
} StmtType;

typedef struct  Stmt {
    StmtType type;
    char *varName;
    Expr *expr;
    // for the if statement
    Expr *ifCondition;
    struct Stmt *thenBranch;
    struct Stmt *elseBranch;
    // for the while statement
    Expr *whileCondition;
    struct Stmt *whileBody;
    // for the block statement
    struct Stmt *body;
    struct Stmt *next;
} Stmt;

typedef struct {
    Lexer lexer;
    Token current;
} Parser;

void init_parser(Parser *parser, const char *input);
Stmt* parse_program(Parser *parser);

#endif // PARSER_H