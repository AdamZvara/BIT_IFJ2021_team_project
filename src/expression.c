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

/**
 * @brief Converts token to a symbol
 */
int token_to_symbol(token_t *token)
{
    token_type_t type = token->type;
    if (type == TOK_LEN) {
        return STR_LEN;
    } else if (type == TOK_MUL) {
        return MUL;
    } else if (type == TOK_DIV) {
        return DIV;
    } else if (type == TOK_INT_DIV) {
        return DIV_INT;
    } else if (type == TOK_PLUS) {
        return PLUS;
    } else if (type == TOK_MINUS) {
        return MINUS;
    } else if (type == TOK_CONCAT) {
        return CONCAT;
    } else if (type == TOK_EQ) {
        return EQ;
    } else if (type == TOK_NEQ) {
        return NOT_EQ;
    } else if (type == TOK_LES) {
        return LESS;
    } else if (type == TOK_LES_EQ) {
        return LESS_EQ;
    } else if (type == TOK_GR) {
        return GREAT;
    } else if (type == TOK_GR_EQ) {
        return GREAT_EQ;
    } else if (type == TOK_LBRACKET) {
        return LEFT_BR;
    } else if (type == TOK_RBRACKET) {
        return RIGHT_BR;
    } else if (type == TOK_ID) {
        return ID;
    } else {
        // token doesn't belong to the expression
        return DOLLAR;
    }
}

prec_table_index_t symbol_to_index(int symbol)
{
    prec_table_index_t rv;
    switch (symbol) {
        case STR_LEN:
            rv = I_STR_LEN;
            break;

        case MUL:
        case DIV:
        case DIV_INT:
            rv = I_MUL_DIV;
            break;

        case PLUS:
        case MINUS:
            rv = I_PLUS_MINUS;
            break;

        case TOK_CONCAT:
            rv = I_CONCAT;
            break;

        case EQ:
        case NOT_EQ:
        case LESS:
        case LESS_EQ:
        case GREAT:
        case GREAT_EQ:
            rv = I_RELATION;
            break;

        case LEFT_BR:
            rv = I_LEFT_BR;
            break;

        case RIGHT_BR:
            rv = I_RIGHT_BR;
            break;

        case ID:
            rv = I_ID;
            break;
    }

    return rv;
}

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

    stack_item_t *top_term;
    int symbol;
    char prec_symbol;

    do {
        ret_val = get_token(new_token);
        if (ret_val) {
            return ret_val;
        }

        top_term = stack_top(&stack_prec);
        if (top_term->data == NON_TERM) {
            // get top terminal
            top_term = top_term->next;
        }
        symbol = token_to_symbol(new_token);

        // get precedence symbol from precedence table
        prec_symbol = prec_table[symbol_to_index(top_term->data)][symbol_to_index(symbol)];
        switch (prec_symbol) {
            case '=':
                stack_push(&stack_prec, symbol);
                GET_NEW_TOKEN(new_token, ret_val);
                break;

            case '<':
                break;

            case '>':
                break;
        }

        // switch and the rest of alg ...

    } while (!end);


    return ret_val;
}
