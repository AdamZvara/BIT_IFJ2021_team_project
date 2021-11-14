/**
 * VUT IFJ Project 2021.
 *
 * @file parser.c
 *
 * @brief Implementation of parser
 *
 * @author Vojtěch Eichler 
 * @author Václav Korvas
 * @author Tomáš Matuš
 * @author Adam Zvara
 */

#include <stdlib.h>
#include <stdio.h>
#include "symtable.h"
#include "scanner.h"
#include "str.h"
#include "error.h"
#include "parser.h"

token_t *curr_token = NULL;
int ret = SUCCESS;

//int token_init(token_t *token) {
//    token = malloc(sizeof(token_t));
//    if (!token)
//        return 1;
//    return 0;
//}

void token_free() {
    FREE_TOK_STRING();
    free(curr_token);
}

int parse()
{
    curr_token = malloc(sizeof(token_t));
    if (!curr_token) {
        return ERROR_INTERNAL;
    }
    ret = require(); 
    

    token_free();
	return ret;
}

int require()
{
    // read new token, should be require keyword, also check for failure
    ret = get_token(curr_token);
    if (ret)
        return ret;
    if ((curr_token->type != TOK_KEYWORD) || (curr_token->attribute.keyword != KW_REQUIRE))
        return ERROR_SYNTAX;

    // check for string after _require_ keyword
    FREE_TOK_STRING();
    ret = get_token(curr_token);
    if (ret)
        return ret;
    if (curr_token->type != (token_type_t)TOK_STRING)
        return ERROR_SYNTAX;

    // go to rule <prog>
    return prog();
}

int prog()
{
    FREE_TOK_STRING();
    ret = get_token(curr_token);
    if (ret)
        return ret;
    if (curr_token->type == TOK_KEYWORD) { // new token is keyword
        if (curr_token->attribute.keyword == KW_GLOBAL) { // check if keyword is _global_
            // get new token that should be ID

            NEXT_TOKEN();
            if (curr_token->type != TOK_ID)
                return ERROR_SYNTAX;
            
            // get new token that should be colon
            NEXT_TOKEN();
            if (curr_token->type != TOK_COLON)
                return ERROR_SYNTAX;

            // get new token that should be keyword _function_
            NEXT_TOKEN();
            if (curr_token->type != TOK_KEYWORD || curr_token->attribute.keyword != KW_FUNCTION)
                return ERROR_SYNTAX;

            // get new token that should be opening bracket
            NEXT_TOKEN();
            if (curr_token->type != TOK_LBRACKET)
                return ERROR_SYNTAX;

            //TODO: call params()

            // get new token that should be closing bracket
            NEXT_TOKEN();
            if (curr_token->type != TOK_RBRACKET)
                return ERROR_SYNTAX;

            //TODO: call ret_params()

            return prog();

        } else if (curr_token->attribute.keyword == KW_FUNCTION) { // check if keyword is _function_
            // get new token that should be ID
            NEXT_TOKEN();
            if (curr_token->type != TOK_ID)
                return ERROR_SYNTAX;

            // get new token that should be opening bracket
            NEXT_TOKEN();
            if (curr_token->type != TOK_LBRACKET)
                return ERROR_SYNTAX;

            //TODO: call params_2()

            // get new token that should be closing bracket
            NEXT_TOKEN();
            if (curr_token->type != TOK_RBRACKET)
                return ERROR_SYNTAX;

            //TODO: call ret_params()

            //TODO: call body()

            // get new token that should be keyword end
            NEXT_TOKEN();
            if (curr_token->type != TOK_KEYWORD || curr_token->attribute.keyword != KW_END)
                return ERROR_SYNTAX;
            
            return prog();

        } else { // unexpected keyword, return error
            return ERROR_SYNTAX;
        }
    } else if (curr_token->type == TOK_ID) { // new token is ID

    } else { // unexpected token, return error
        return ERROR_SYNTAX;
    }
}

int main() {
    int ret_main = parse();
    return ret_main;
}
