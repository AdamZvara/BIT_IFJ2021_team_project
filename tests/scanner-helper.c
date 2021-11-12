#include <stdio.h>
#include <stdlib.h>
#include "../src/scanner.h"
#include "../src/error.h"

/* Compile like this while in the same dir as this file:
 * gcc -o scanner-helper scanner-helper.c ../scanner.c ../str.c */

int main ()
{
    // Load first token, then read tokens and output information based
    // on the type of the token until TOK_EOF is read
    token_t *token = malloc(sizeof(token_t));

    get_token(token);
    while(token->type != TOK_EOF) {
        switch (token->type) {
            case TOK_GR:
                printf("TOK_GR\n");
                break;
            case TOK_MINUS:
                printf("TOK_MINUS\n");
                break;
            case TOK_INT:
                printf("TOK_INT : ");
                printf("%d\n", token->attribute.number);
                break;
            case TOK_DECIMAL:
                printf("TOK_DECIMAL : ");
                printf("%f\n", token->attribute.decimal);
                break;
            case TOK_STRING:
                printf("TOK_STRING : ");
                printf("%s\n", token->attribute.s.str);
                break;
            case TOK_ID:
                printf("TOK_ID : ");
                printf("%s\n", token->attribute.s.str);
                break;
            case TOK_KEYWORD:
                printf("TOK_KEYWORD : ");
                switch (token->attribute.keyword) {
                    case KW_DO:
                        printf("do\n");
                        break;
                    case KW_ELSE:
                        printf("else\n");
                        break;
                    case KW_END:
                        printf("end\n");
                        break;
                    case KW_FUNCTION:
                        printf("function\n");
                        break;
                    case KW_GLOBAL:
                        printf("global\n");
                        break;
                    case KW_IF:
                        printf("if\n");
                        break;
                    case KW_INTEGER:
                        printf("integer\n");
                        break;
                    case KW_LOCAL:
                        printf("local\n");
                        break;
                    case KW_NIL:
                        printf("nil\n");
                        break;
                    case KW_NUMBER:
                        printf("number\n");
                        break;
                    case KW_REQUIRE:
                        printf("require\n");
                        break;
                    case KW_RETURN:
                        printf("return\n");
                        break;
                    case KW_STRING:
                        printf("string\n");
                        break;
                    case KW_THEN:
                        printf("then\n");
                        break;
                    case KW_WHILE:
                        printf("while\n");
                        break;
                }
                break;
            /*case TOK_EOL:
                printf("TOK_EOL\n");
                break;
            case TOK_NOTHING:
                printf("TOK_NOTHING\n");
                break;*/
            case TOK_EQ:
                printf("TOK_EQ\n");
                break;
            case TOK_NEQ:
                printf("TOK_NEQ\n");
                break;
            case TOK_LBRACKET:
                printf("TOK_LBRACKET\n");
                break;
            case TOK_RBRACKET:
                printf("TOK_RBRACKET\n");
                break;
            case TOK_PLUS:
                printf("TOK_PLUS\n");
                break;
            case TOK_LEN:
                printf("TOK_LEN\n");
                break;
            case TOK_MUL:
                printf("TOK_MUL\n");
                break;
            case TOK_COLON:
                printf("TOK_COLON\n");
                break;
            case TOK_COMMA:
                printf("TOK_COMMA\n");
                break;
            case TOK_EOF:
                printf("TOK_EOF\n");
                break;
            case TOK_INT_DIV:
                printf("TOK_INT_DIV\n");
                break;
            case TOK_DIV:
                printf("TOK_DIV\n");
                break;
            case TOK_CONCAT:
                printf("TOK_CONCAT\n");
                break;
            case TOK_GR_EQ:
                printf("TOK_GR_EQ\n");
                break;
            case TOK_LES_EQ:
                printf("TOK_LES_EQ\n");
                break;
            case TOK_LES:
                printf("TOK_LES\n");
                break;
            case TOK_ASSIGN:
                printf("TOK_ASSIGN\n");
                break;
            default:
                break;

        }
        if (get_token(token) != SUCCESS) {
            printf("ERROR\n");
        };
    }
    return 0;
}
