/**
 * VUT IFJ Project 2021.
 *
 * @file parser.h
 *
 * @brief Definition of parser
 *
 * @author Vojtěch Eichler 
 * @author Václav Korvas
 * @author Tomáš Matuš
 * @author Adam Zvara
 */

#ifndef _PARSER_H_
#define _PARSER_H_

#include "symtable.h"
#include "scanner.h"
#include "str.h"
#include "error.h"

#define FREE_TOK_STRING() \
    do {               \
		if (curr_token->type == TOK_STRING || curr_token->type == TOK_ID) \
			str_free(&curr_token->attribute.s); \
    } while(0);          \

#define NEXT_TOKEN() \
    do  {             \
        FREE_TOK_STRING()    \
        ret = get_token(curr_token); \
        if (ret) {     \
            return ret; \
        }    \
    } while(0); \

int parse();
int require();
int prog();
int params();
int params_n();
int params_2();
int params_2_n();
int ret_params();
int ret_params_n();
int body();
int body_n();
int assign_single();
int assign_multi();
int r_side();
int r_side_n();
int func();
int init();
int init_n();
int args();
int args_n();
int term();
int types_keyword();

#endif
