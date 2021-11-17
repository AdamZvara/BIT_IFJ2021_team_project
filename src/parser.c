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
token_t *backup_token = NULL;
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
    //TODO: check if we have backup token, if there is one, dont read new token and
    // just copy backup_token to curr_token and make backup_token NULL
    if (!backup_token) {
        NEXT_TOKEN();
    } else {
        curr_token = backup_token;
        backup_token = NULL;
    }
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

            // call params rule, check exit code and return if params were not successful,
            // also skip reading next token
            ret = params();
            if (ret)
                return ret;

            // step into <ret_params> rule
            ret = ret_params();
            if (ret)
                return ret;

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
            ret = params_2();
            if (ret)
                return ret;

            // step into <ret_params> rule
            ret = ret_params();
            if (ret)
                return ret;
            
            //TODO: call body()
            // body has to check for end
            // ret = body()
            // if (ret)
            //     return ret;

            return prog();

        } else { // unexpected keyword, return error
            return ERROR_SYNTAX;
        }
    } else if (curr_token->type == TOK_ID) { // new token is ID
        NEXT_TOKEN();
        if (curr_token->type != TOK_LBRACKET)
            return ERROR_SYNTAX;

        //TODO: call args()
        
        NEXT_TOKEN();
        if (curr_token->type != TOK_RBRACKET)
            return ERROR_SYNTAX;

        return prog();

    }  else if (curr_token->type == TOK_EOF) {
        // end of input, start returning all the way up the recursion
        return ret;
    } else { // unexpected token, return error
        return ERROR_SYNTAX;
    }
}

int params() {
    NEXT_TOKEN();
    if (curr_token->type == TOK_RBRACKET) {
        return ret;
    } else if ((curr_token->type == TOK_KEYWORD && curr_token->attribute.keyword == KW_STRING) ||
               (curr_token->type == TOK_KEYWORD && curr_token->attribute.keyword == KW_NUMBER) ||
               (curr_token->type == TOK_KEYWORD && curr_token->attribute.keyword == KW_INTEGER)) {

        return params_n();
    } else {
        return ERROR_SYNTAX;
    }
}

int params_n() {
    NEXT_TOKEN();
    if (curr_token->type == TOK_RBRACKET) {
        return ret;
    } else if (curr_token->type == TOK_COMMA) {
        NEXT_TOKEN();
        if ((curr_token->type == TOK_KEYWORD && curr_token->attribute.keyword == KW_STRING) ||
               (curr_token->type == TOK_KEYWORD && curr_token->attribute.keyword == KW_NUMBER) ||
               (curr_token->type == TOK_KEYWORD && curr_token->attribute.keyword == KW_INTEGER)) {

            return params_n();
        } else {
            return ERROR_SYNTAX;
        }
    } else {
        return ERROR_SYNTAX;
    }
}

int params_2() {
    // first check if params are empty or not
    NEXT_TOKEN();
    if (curr_token->type == TOK_RBRACKET) {
        return ret;
    } else if (curr_token->type == TOK_ID) { // params start correctly with ID
        // now check the rest of the syntax and go to params_2_n if everything is correct
        NEXT_TOKEN();
        if (curr_token->type != TOK_COLON)
            return ERROR_SYNTAX;
        // next few lines of code check <types_keyword> rule
        NEXT_TOKEN();
        if ((curr_token->type == TOK_KEYWORD && curr_token->attribute.keyword == KW_STRING) ||
               (curr_token->type == TOK_KEYWORD && curr_token->attribute.keyword == KW_NUMBER) ||
               (curr_token->type == TOK_KEYWORD && curr_token->attribute.keyword == KW_INTEGER)) {

            return params_2_n();
        } else {
            return ERROR_SYNTAX;
        }
    } else {
        return ERROR_SYNTAX;
    }
}

int params_2_n() {
    // check for end of params
    NEXT_TOKEN();
    if (curr_token->type == TOK_RBRACKET) {
        return ret;
    } else if (curr_token->type == TOK_COMMA) { // check for comma
        // check the rest of the syntax and descend into parama_2_n if everything is correct
        NEXT_TOKEN();
        if (curr_token->type != TOK_ID)
            return ERROR_SYNTAX;
        NEXT_TOKEN();
        if (curr_token->type != TOK_COLON)
            return ERROR_SYNTAX;
        // next lines check <types_keyword> rule
        NEXT_TOKEN();
        if ((curr_token->type == TOK_KEYWORD && curr_token->attribute.keyword == KW_STRING) ||
               (curr_token->type == TOK_KEYWORD && curr_token->attribute.keyword == KW_NUMBER) ||
               (curr_token->type == TOK_KEYWORD && curr_token->attribute.keyword == KW_INTEGER)) {

            return params_2_n();
        } else {
            return ERROR_SYNTAX;
        }
    } else {
        return ERROR_SYNTAX;
    }
}

int ret_params() {
    NEXT_TOKEN();
    if (curr_token->type != TOK_COLON) {
        backup_token = curr_token;
        return ret;
    }
    NEXT_TOKEN();
    if ((curr_token->type == TOK_KEYWORD && curr_token->attribute.keyword == KW_STRING) ||
           (curr_token->type == TOK_KEYWORD && curr_token->attribute.keyword == KW_NUMBER) ||
           (curr_token->type == TOK_KEYWORD && curr_token->attribute.keyword == KW_INTEGER)) {

        return ret_params_n();
    } else {
        return ERROR_SYNTAX;
    }
}

int ret_params_n() {
    NEXT_TOKEN();
    if (curr_token->type != TOK_COMMA) {
        backup_token = curr_token;
        return ret;
    }
    NEXT_TOKEN();
    if ((curr_token->type == TOK_KEYWORD && curr_token->attribute.keyword == KW_STRING) ||
           (curr_token->type == TOK_KEYWORD && curr_token->attribute.keyword == KW_NUMBER) ||
           (curr_token->type == TOK_KEYWORD && curr_token->attribute.keyword == KW_INTEGER)) {

        return ret_params_n();
    } else {
        return ERROR_SYNTAX;
    }
}

int body() {
    //TODO: check if we have backup token, if there is one, dont read new token and
    // just copy backup_token to curr_token and make backup_token NULL

}

int main() {
    int ret_main = parse();
    return ret_main;
}
