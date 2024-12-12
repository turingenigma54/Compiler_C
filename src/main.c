#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "parser.h"

typedef struct {
    char var[64];
    int value;
} Var;

static Var vars[100];
static int var_count = 0;

static int get_var_value(const char* name) {
    for (int i = 0; i < var_count; i++) {
        if (strcmp(vars[i].var, name) == 0)
            return vars[i].value;
    }
    return 0;
}

static void set_var_value(const char* name, int val) {
    for (int i = 0; i < var_count; i++) {
        if (strcmp(vars[i].var, name) == 0) {
            vars[i].value = val;
            return;
        }
    }
    strncpy(vars[var_count].var, name, 63);
    vars[var_count].var[63] = '\0';
    vars[var_count].value = val;
    var_count++;
}

static int eval_expr(Expr *e);

static void eval_statements(Stmt *stmt);

static int eval_expr(Expr *e) {
    switch (e->type) {
        case EXPR_NUMBER:
            return e->value;
        case EXPR_IDENTIFIER:
            return get_var_value(e->name);
        case EXPR_STRING:
            return 0;
        case EXPR_BINARY: {
            int left = eval_expr(e->left);
            int right = eval_expr(e->right);
            switch (e->op.type) {
                case TOKEN_PLUS: return left + right;
                case TOKEN_MINUS: return left - right;
                case TOKEN_STAR: return left * right;
                case TOKEN_SLASH: return left / right;
                case TOKEN_PERCENT: return left % right;
                case TOKEN_EQEQ: return (left == right) ? 1 : 0;
                case TOKEN_LE: return (left <= right) ? 1 : 0;
                default: return 0;
            }
        }
    }
    return 0;
}

static void eval_stmt(Stmt *s) {
    switch (s->type) {
        case STMT_ASSIGN: {
            int val = eval_expr(s->expr);
            set_var_value(s->varName, val);
            break;
        }
        case STMT_PRINT: {
            if (s->expr->type == EXPR_IDENTIFIER) {
                printf("%d\n", get_var_value(s->expr->name));
            } else if (s->expr->type == EXPR_STRING) {
                printf("%s\n", s->expr->stringVal);
            } else {
                printf("%d\n", eval_expr(s->expr));
            }
            break;
        }
        case STMT_IF: {
            int cond = eval_expr(s->ifCondition);
            if (cond != 0) {
                eval_statements(s->thenBranch);
            } else if (s->elseBranch) {
                eval_statements(s->elseBranch);
            }
            break;
        }
        case STMT_WHILE: {
            while (eval_expr(s->whileCondition) != 0) {
                eval_statements(s->whileBody);
            }
            break;
        }
        case STMT_BLOCK: {
            eval_statements(s->body);
            break;
        }
    }
}

static void eval_statements(Stmt *stmt) {
    for (Stmt *s = stmt; s; s = s->next) {
        eval_stmt(s);
    }
}

int main(void) {
    const char *source =
        "i = 1;\n"
        "while (i <= 15) {\n"
        "   if (i % 3 == 0) {\n"
        "       if (i % 5 == 0) {\n"
        "           print \"FizzBuzz\";\n"
        "       } else {\n"
        "           print \"Fizz\";\n"
        "       }\n"
        "   } else {\n"
        "       if (i % 5 == 0) {\n"
        "           print \"Buzz\";\n"
        "       } else {\n"
        "           print i;\n"
        "       }\n"
        "   }\n"
        "   i = i + 1;\n"
        "}\n";

    Parser parser;
    init_parser(&parser, source);
    Stmt* program = parse_program(&parser);

    eval_statements(program);

    return 0;
}
