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
            
            ret = body();
            if (ret)
                 return ret;

            return prog();

        } else { // unexpected keyword, return error
            return ERROR_SYNTAX;
        }
    } else if (curr_token->type == TOK_ID) { // new token is ID
        NEXT_TOKEN();
        if (curr_token->type != TOK_LBRACKET)
            return ERROR_SYNTAX;

        //TODO: call args()
        
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
    if (!backup_token) {
        NEXT_TOKEN();
    } else {
        curr_token = backup_token;
        backup_token = NULL;
    }
    
    if (curr_token->type == TOK_KEYWORD) {
        switch (curr_token->attribute.keyword) {
            case KW_LOCAL:
                NEXT_TOKEN();
                if (curr_token->type != TOK_ID)
                    return ERROR_SYNTAX;
                NEXT_TOKEN();
                if (curr_token->type != TOK_COLON)
                    return ERROR_SYNTAX;
                NEXT_TOKEN();
                if ((curr_token->type == TOK_KEYWORD && curr_token->attribute.keyword == KW_STRING) ||
                   (curr_token->type == TOK_KEYWORD && curr_token->attribute.keyword == KW_NUMBER) ||
                   (curr_token->type == TOK_KEYWORD && curr_token->attribute.keyword == KW_INTEGER)) {

                    ret = init();
                    if (ret)
                        return ret;
                }
                return body();
                break;
            case KW_IF: // IF <expr> THEN <body> ELSE <body> END <body>
                //TODO: call expr()

                // THEN
                NEXT_TOKEN();
                if (curr_token->type != TOK_KEYWORD || curr_token->attribute.keyword != KW_THEN)
                    return ERROR_SYNTAX;

                // <body>
                ret = body();
                if (ret)
                    return ret;

                // ELSE
                NEXT_TOKEN();
                if (curr_token->type != TOK_KEYWORD || curr_token->attribute.keyword != KW_ELSE)
                    return ERROR_SYNTAX;
                
                // <body>
                ret = body();
                if (ret)
                    return ret;

                // END
                NEXT_TOKEN();
                if (curr_token->type != TOK_KEYWORD || curr_token->attribute.keyword != KW_END)
                    return ERROR_SYNTAX;
                
                // <body>
                return body();
                break;
            case KW_WHILE:
                //TODO: call expr()
                
                // DO
                NEXT_TOKEN();
                if (curr_token->type != TOK_KEYWORD || curr_token->attribute.keyword != KW_DO)
                    return ERROR_SYNTAX;
                
                // <body>
                ret = body();
                if (ret)
                    return ret;

                // END
                NEXT_TOKEN();
                if (curr_token->type != TOK_KEYWORD || curr_token->attribute.keyword != KW_END)
                    return ERROR_SYNTAX;
                
                // <body>
                return body();
                break;
            case KW_END:
                return ret;
                break;
            case KW_RETURN:
                return r_side();
                break;
            default:
                return ERROR_SYNTAX;
                break;
        }
    } else if (curr_token->type == TOK_ID) { // ID <body_n> <body>
        ret = body_n();
        if (ret)
            return ret;
        return body();
    } else {
        return ERROR_SYNTAX;
    }
}

int body_n() {
    NEXT_TOKEN();
    if (curr_token->type == TOK_LBRACKET) {
        //TODO: call args()
        //ret = args();
        //if (ret)
        //    return ret;

        return ret;
    } else if (curr_token->type == TOK_EQ) {
        return assign_single();
    } else if (curr_token->type == TOK_COMMA) {
        NEXT_TOKEN();
        if (curr_token->type != TOK_ID)
            return ERROR_SYNTAX;

        ret = assign_multi();
        if (ret)
            return ret;
        
        
        return r_side();
    } else {
        return ERROR_SYNTAX;
    }
}

int assign_single() {
    // call expression()
    // int exp_ret = expression();
    // if (exp_ret == SUCCESS)
    //    return exp_ret;
    // else if (exp_ret == FUNCTION)
    //    return func();
    // else
    //    return ERROR_SYNTAX;
    return 0;
}

int assign_multi() {
    NEXT_TOKEN();
    if (curr_token->type == TOK_COMMA) {
        NEXT_TOKEN();
        if (curr_token->type != TOK_ID)
            return ERROR_SYNTAX;
        return assign_multi();
    } else if (curr_token->type == TOK_EQ) {
        return ret;
    } else {
        return ERROR_SYNTAX;
    }
}

int r_side() {
    // call expression()
    // int exp_ret = expression();
    // if (exp_ret == SUCCESS) {
    //    return r_side_n();
    //    
    // else if (exp_ret == FUNCTION)
    //    ret = func();
    //    if (ret) 
    //        return ret;
    //    return r_side_n();
    // else
    //    return exp_ret;
    

    //TMP
    return 0;
}

int r_side_n() {
    NEXT_TOKEN();
    if (curr_token->type == TOK_COMMA) {
        return r_side();
    } else {
        backup_token = curr_token;
        return ret;
    }
}

int func() {
    // <args>
    ret = args();
    if (ret)
        return ret;

    return ret;
}

int init() {
    NEXT_TOKEN();
    if (curr_token->type == TOK_EQ) {
        return init_n();
    } else {
        backup_token = curr_token;
        return ret;
    }
}

int init_n() {
    // call expression()
    // int exp_ret = expression();
    // if (exp_ret == SUCCESS)
    //    return exp_ret;
    // else if (exp_ret == FUNCTION)
    //    return func();
    // else
    //    return ERROR_SYNTAX;


    // TMP
    return 0;
}

int args() {
    NEXT_TOKEN();
    if (curr_token->type == TOK_RBRACKET) {
        return ret;
    } else {
        if (curr_token->type == TOK_STRING || curr_token->type == TOK_DECIMAL ||
            curr_token->type == TOK_INT    || curr_token->type == TOK_ID) {

            return args_n();
        } else {
            return ERROR_SYNTAX;
        }
    }
}

int args_n() {
    NEXT_TOKEN();
    if (curr_token->type == TOK_COMMA) {
        return args();
    } else if (curr_token->type == TOK_RBRACKET) {
        return ret;
    } else {
        return ERROR_SYNTAX;
    }

}


int main() {
    int ret_main = parse();
    return ret_main;
}
