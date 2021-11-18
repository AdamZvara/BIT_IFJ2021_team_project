#include <stdio.h>
#include <stdlib.h>

#include "stack.h"
#include "expression.h"
#include "scanner.h"
#include "symtable.h"
#include "error.h"
#include "parser.h"

const char prec_table[TABLE_SIZE][TABLE_SIZE] = {
    // ---> current token
    //| # |/*//| +- | .. |  r | (  |  ) |  id | $                | what is on stack_top
    { '-', '-', '-', '-' , '-', '<', '-', '<', '>' }, // #       |
    { '<', '>', '>', '-' , '>', '<', '>', '<', '>' }, // /*//    v
    { '<', '<', '>', '-' , '>', '<', '>', '<', '>' }, // +-
    { '-', '-', '-', '<' , '>', '<', '>', '<', '>' }, // ..
    { '<', '<', '<', '<' , '-', '<', '>', '<', '>' }, // r ( < <= > >= )
    { '<', '<', '<', '<' , '<', '<', '=', '<', '-' }, // (
    { '>', '>', '>', '>' , '>', '-', '>', '-', '>' }, // )
    { '-', '>', '>', '>' , '>', '-', '>', '-', '>' }, // id
    { '<', '<', '<', '<' , '<', '<', '-', '<', '-' }, // $
};

int ret_val = EC_SUCCESS;
int end = 0;

int expression(token_t *return_token)
{
    // init stack and push $
    stack_t stack_prec;
    stack_init(&stack_prec);
    stack_push(&stack_prec, DOLLAR);

    token_t *new_token = malloc(sizeof(token_t));
    if (!new_token) {
        return ERROR_INTERNAL;
    }

    ret_val = get_token(new_token);
    if (ret_val) {
        return ret_val;
    }

    while (!end) {
    }

    return ret_val;
}
