#ifndef _EXPRESSION_H_
#define _EXPRESSION_H_

#include <stdio.h>
#include <stdlib.h>

#include "stack.h"
#include "scanner.h"
#include "symtable.h"
#include "error.h"
#include "parser.h"
#include "str.h"

#define TABLE_SIZE 9
// return values
#define EC_SUCCESS 0
#define EC_SYNTAX 1
#define EC_FUNC 2

#define FREE_STRING_TOKEN(token) \
    do { \
        if (token->type == TOK_STRING || token->type == TOK_ID) \
            str_free(&token->attribute.s); \
    } while(0);

#define GET_NEW_TOKEN(token, ret) \
    do {             \
        FREE_STRING_TOKEN(token)    \
        ret = get_token(token); \
        if (ret) {     \
            return ret; \
        }    \
    } while(0);

typedef enum
{
    // #
    STR_LEN,

    // * / '//'
    MUL,
    DIV,
    DIV_INT,

    // + -
    PLUS,
    MINUS,

    // ..
    CONCAT,

    // = ~= < <= > >=
    EQ,
    NOT_EQ,
    LESS,
    LESS_EQ,
    GREAT,
    GREAT_EQ,

    // (
    LEFT_BR,

    // )
    RIGHT_BR,

    // id
    ID,

    // $
    DOLLAR,

    // <
    HANDLE,

    NON_TERM
} prec_table_term_t;

typedef enum
{
    I_STR_LEN,
    I_MUL_DIV,
    I_PLUS_MINUS,
    I_CONCAT,
    I_RELATION,
    I_LEFT_BR,
    I_RIGHT_BR,
    I_ID,
    I_DOLLAR
} prec_table_index_t;


#endif // _EXPRESSION_H_
