#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "parser.h"

static char* my_strndup(const char *s, size_t n) {
    size_t len = strlen(s);
    if (n > len) n = len;
    char *result = (char *)malloc(n+1);
    if (!result) return NULL;
    memcpy(result, s, n);
    result[n] = '\0';
    return result;
}

static void advance(Parser *p) {
    p->current = get_next_token(&p->lexer);
    printf("advance: current token = %d\n", p->current.type);
}

static int match(Parser *p, TokenType type) {
    if (p->current.type == type) {
        printf("match: matched token %d\n", type);
        advance(p);
        return 1;
    }
    printf("match: expected token %d but got %d\n", type, p->current.type);
    return 0;
}

static Expr* new_expr_number(int value) {
    Expr* e = calloc(1, sizeof(Expr));
    e->type = EXPR_NUMBER;
    e->value = value;
    return e;
}

static Expr* new_expr_identifier(const char *start, int length) {
    Expr* e = calloc(1, sizeof(Expr));
    e->type = EXPR_IDENTIFIER;
    e->name = my_strndup(start, length);
    return e;
}

static Expr* new_expr_binary(Expr* left, Token op, Expr* right) {
    Expr* e = calloc(1, sizeof(Expr));
    e->type = EXPR_BINARY;
    e->left = left;
    e->right = right;
    e->op = op;
    return e;
}

static Expr* parse_expression(Parser *p);
static Stmt* parse_statement(Parser *p);

static Expr* parse_primary(Parser *p) {
    printf("parse_primary: current token = %d\n", p->current.type);

    if (p->current.type == TOKEN_NUMBER) {
        int val = atoi(p->current.start);
        Expr* e = new_expr_number(val);
        advance(p);
        return e;
    } else if (p->current.type == TOKEN_IDENTIFIER) {
        Expr* e = new_expr_identifier(p->current.start, p->current.length);
        advance(p);
        return e;
    } else if (p->current.type == TOKEN_STRING) {
        Expr *e = calloc(1, sizeof(Expr));
        e->type = EXPR_STRING;
        e->stringVal = my_strndup(p->current.start, p->current.length);
        advance(p);
        return e;
    } else if (p->current.type == TOKEN_LPAREN) {
        advance(p);
        Expr* e = parse_expression(p);
        match(p, TOKEN_RPAREN);
        return e;
    }

    printf("parse_primary: returning NULL (unexpected token %d)\n", p->current.type);
    return NULL;
}

static Expr* parse_factor(Parser *p) {
    Expr *expr = parse_primary(p);
    while (p->current.type == TOKEN_STAR || p->current.type == TOKEN_SLASH || p->current.type == TOKEN_PERCENT) {
        Token op = p->current;
        advance(p);
        Expr *right = parse_primary(p);
        expr = new_expr_binary(expr, op, right);
    }
    return expr;
}

static Expr* parse_term(Parser *p) {
    Expr* expr = parse_factor(p);
    while (p->current.type == TOKEN_PLUS || p->current.type == TOKEN_MINUS) {
        Token op = p->current;
        advance(p);
        Expr *right = parse_factor(p);
        expr = new_expr_binary(expr, op, right);
    }
    return expr;
}

static Expr* parse_comparison(Parser *p) {
    Expr* expr = parse_term(p);
    while (p->current.type == TOKEN_EQEQ || p->current.type == TOKEN_LE) {
        Token op = p->current;
        advance(p);
        Expr *right = parse_term(p);
        expr = new_expr_binary(expr, op, right);
    }
    return expr;
}

static Expr* parse_expression(Parser *p) {
    printf("parse_expression\n");
    return parse_comparison(p);
}

static Stmt* new_stmt(StmtType type) {
    Stmt* s = calloc(1, sizeof(Stmt));
    s->type = type;
    return s;
}

static Stmt* parse_block(Parser *p) {
    printf("parse_block: expecting '{', current token = %d\n", p->current.type);
    match(p, TOKEN_LBRACE);
    Stmt *head = NULL;
    Stmt *tail = NULL;
    while (p->current.type != TOKEN_RBRACE && p->current.type != TOKEN_EOF) {
        printf("parse_block: parsing inner statement\n");
        Stmt *s = parse_statement(p);
        if (!s) {
            printf("parse_block: parse_statement returned NULL!\n");
            break; // Prevent infinite loop, just break on error
        }
        if (!head) head = s; else tail->next = s;
        tail = s;
    }
    if (p->current.type == TOKEN_RBRACE) {
        match(p, TOKEN_RBRACE);
    } else {
        printf("parse_block: missing '}' token\n");
    }
    Stmt *blockStmt = new_stmt(STMT_BLOCK);
    blockStmt->body = head;
    return blockStmt;
}

static Stmt* parse_if_statement(Parser *p) {
    printf("parse_if_statement: current token = %d\n", p->current.type);
    match(p, TOKEN_IF);
    match(p, TOKEN_LPAREN);
    Expr *condition = parse_expression(p);
    match(p, TOKEN_RPAREN);

    Stmt *s = new_stmt(STMT_IF);
    s->ifCondition = condition;
    Stmt *thenBlock = parse_block(p);
    s->thenBranch = thenBlock->body;
    if (p->current.type == TOKEN_ELSE) {
        advance(p);
        Stmt *elseBlock = parse_block(p);
        s->elseBranch = elseBlock->body;
    }
    return s;
}

static Stmt* parse_while_statement(Parser *p) {
    printf("parse_while_statement: current token = %d\n", p->current.type);
    match(p, TOKEN_WHILE);
    match(p, TOKEN_LPAREN);
    Expr *condition = parse_expression(p);
    match(p, TOKEN_RPAREN);

    Stmt *s = new_stmt(STMT_WHILE);
    Stmt *bodyBlock = parse_block(p);
    s->whileCondition = condition;
    s->whileBody = bodyBlock->body;
    return s;
}

static Stmt* parse_statement(Parser *p) {
    printf("parse_statement: current token = %d\n", p->current.type);

    if (p->current.type == TOKEN_IDENTIFIER) {
        char *name = my_strndup(p->current.start, p->current.length);
        advance(p);
        match(p, TOKEN_ASSIGN);
        Expr* expr = parse_expression(p);
        match(p, TOKEN_SEMICOLON);
        Stmt *s = new_stmt(STMT_ASSIGN);
        s->varName = name;
        s->expr = expr;
        return s;
    } else if (p->current.type == TOKEN_PRINT) {
        advance(p);
        Expr *arg = NULL;
        if (p->current.type == TOKEN_IDENTIFIER) {
            arg = new_expr_identifier(p->current.start, p->current.length);
            advance(p);
        } else if (p->current.type == TOKEN_STRING) {
            arg = calloc(1, sizeof(Expr));
            arg->type = EXPR_STRING;
            arg->stringVal = my_strndup(p->current.start, p->current.length);
            advance(p);
        }
        match(p, TOKEN_SEMICOLON);
        Stmt *s = new_stmt(STMT_PRINT);
        s->expr = arg;
        return s;
    } else if (p->current.type == TOKEN_IF) {
        return parse_if_statement(p);
    } else if (p->current.type == TOKEN_WHILE) {
        return parse_while_statement(p);
    } else if (p->current.type == TOKEN_LBRACE) {
        return parse_block(p);
    }

    printf("parse_statement: returning NULL (unrecognized token %d)\n", p->current.type);
    return NULL;
}

void init_parser(Parser *parser, const char *input) {
    init_lexer(&parser->lexer, input);
    advance(parser);
}

Stmt* parse_program(Parser *parser) {
    printf("parse_program: start\n");
    Stmt *head = NULL;
    Stmt *tail = NULL;
    while (parser->current.type != TOKEN_EOF) {
        printf("parse_program: about to parse a statement, current token = %d\n", parser->current.type);
        Stmt* s = parse_statement(parser);
        if (!s) {
            printf("parse_program: parse_statement returned NULL, stopping.\n");
            break;
        }
        if (!head) head = s; else tail->next = s;
        tail = s;
    }
    printf("parse_program: end\n");
    return head;
}
