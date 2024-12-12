#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include "lexer.h"

void init_lexer(Lexer *lexer, const char *input) {
    lexer->input = input;
    lexer->pos = 0;
}

static char peek(Lexer *lexer) {
    return lexer->input[lexer->pos];
}

static char advance(Lexer *lexer) {
    return lexer->input[lexer->pos++];
}

static void skip_whitespace(Lexer *lexer) {
    while (isspace(peek(lexer))) advance(lexer);
}

Token get_next_token(Lexer *lexer) {
    skip_whitespace(lexer);
    Token token;
    token.start = &lexer->input[lexer->pos];
    char c = peek(lexer);

    if (c == '\0') {
        token.type = TOKEN_EOF;
        token.length = 0;
        return token;
    }

    // Identifiers and keywords
    if (isalpha(c)) {
        int start_pos = lexer->pos;
        while (isalnum(peek(lexer))) advance(lexer);
        int length = lexer->pos - start_pos;
        const char *text = &lexer->input[start_pos];

        if (length == 5 && strncmp(text, "print", 5) == 0) {
            token.type = TOKEN_PRINT;
        } else if (length == 2 && strncmp(text, "if", 2) == 0) {
            token.type = TOKEN_IF;
        } else if (length == 4 && strncmp(text, "else", 4) == 0) {
            token.type = TOKEN_ELSE;
        } else if (length == 5 && strncmp(text, "while", 5) == 0) {
            token.type = TOKEN_WHILE;
        } else {
            token.type = TOKEN_IDENTIFIER;
        }
        token.length = length;
        return token;
    }

    // Numbers
    if (isdigit(c)) {
        int start_pos = lexer->pos;
        while (isdigit(peek(lexer))) advance(lexer);
        token.type = TOKEN_NUMBER;
        token.length = lexer->pos - start_pos;
        return token;
    }

    // Handle other characters
    switch (c) {
        case '=':
            advance(lexer);
            if (peek(lexer) == '=') {
                advance(lexer);
                token.type = TOKEN_EQEQ;
                token.length = 2;
            } else {
                token.type = TOKEN_ASSIGN;
                token.length = 1;
            }
            return token;

        case '<':
            advance(lexer);
            if (peek(lexer) == '=') {
                advance(lexer);
                token.type = TOKEN_LE;
                token.length = 2;
            } else {
                token.type = TOKEN_UNKNOWN;
                token.length = 1;
            }
            return token;

        case '%':
            advance(lexer);
            token.type = TOKEN_PERCENT;
            token.length = 1;
            return token;

        case '+':
            advance(lexer); 
            token.type = TOKEN_PLUS; 
            token.length = 1; 
            return token;

        case '-':
            advance(lexer); 
            token.type = TOKEN_MINUS; 
            token.length = 1; 
            return token;

        case '*':
            advance(lexer); 
            token.type = TOKEN_STAR; 
            token.length = 1; 
            return token;

        case '/':
            advance(lexer); 
            token.type = TOKEN_SLASH; 
            token.length = 1; 
            return token;

        case ';':
            advance(lexer); 
            token.type = TOKEN_SEMICOLON; 
            token.length = 1; 
            return token;

        case '(':
            advance(lexer); 
            token.type = TOKEN_LPAREN; 
            token.length = 1; 
            return token;

        case ')':
            advance(lexer); 
            token.type = TOKEN_RPAREN; 
            token.length = 1; 
            return token;

        case '{':
            advance(lexer); 
            token.type = TOKEN_LBRACE; 
            token.length = 1; 
            return token;

        case '}':
            advance(lexer); 
            token.type = TOKEN_RBRACE; 
            token.length = 1; 
            return token;

        case '"': {
            // Handle string literals
            advance(lexer); // consume the opening quote
            const char *start_str = &lexer->input[lexer->pos];
            while (peek(lexer) != '"' && peek(lexer) != '\0') {
                advance(lexer);
            }

            if (peek(lexer) == '"') {
                // Found closing quote
                int str_len = &lexer->input[lexer->pos] - start_str;
                advance(lexer); // consume closing quote
                token.type = TOKEN_STRING;
                token.start = start_str;
                token.length = str_len;
                return token;
            } else {
                // Unterminated string
                token.type = TOKEN_UNKNOWN;
                token.length = 0;
                printf("Unknown character or unterminated string\n");
                return token;
            }
        }

        default:
            // Unknown character
            advance(lexer);
            token.type = TOKEN_UNKNOWN;
            token.length = 1;
            printf("Unknown character: '%c' (ASCII: %d)\n", c, (int)c);
            return token;
    }
}
