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

#include "func_helper.h"

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

#define GET_ID curr_token->attribute.s
#define GET_KW curr_token->attribute.keyword
#define GET_TYPE curr_token->type

int parse();
int require();
int prog();
int params(func_def_t *f_helper);
int params_n(func_def_t *f_helper);
int params_2(func_def_t *f_helper);
int params_2_n(func_def_t *f_helper);
int ret_params(func_def_t *f_helper);
int ret_params_n(func_def_t *f_helper);
int body();
int body_n(func_def_t *f_helper);
int assign_single(func_def_t *f_helper);
int assign_multi();
int r_side();
int r_side_n();
int func();
int init(struct local_data *id);
int init_n(struct local_data *id);
int args(func_def_t *f_helper);
int args_n(func_def_t *f_helper);
int term();
int types_keyword();

#endif
