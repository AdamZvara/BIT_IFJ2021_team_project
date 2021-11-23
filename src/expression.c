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
    { '-', '>', '>', '>' , '>', '-', '>', '>', '>' }, // id
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

        case DOLLAR:
            rv = I_DOLLAR;
            break;
    }

    return rv;
}

int reduce(stack_t *stack)
{
    int count = items_to_handle(stack);
    stack_item_t *top = stack_top(stack);

    if (count == 1) {
        if (top->data == ID) {
            // ID
            // Push ID
        } else {
            return ERROR_SYNTAX;
        }
    } else if (count == 2 ) {
        if (top->data == NON_TERM && top->next->data == STR_LEN){
            // unary operator #
            // Generate unary operation #
        } else {
            return ERROR_SYNTAX;
        }
    } else if (count == 3 ) {
        if (top->data == NON_TERM && top->next->next->data == NON_TERM) {
            // binary operators
            // Generate binary operation
            // TODO brackets
        } else {
            return ERROR_SYNTAX;
        }
    } else {
        return ERROR_SYNTAX;
    }

    // replace rule with NON_TERM
    for (int i = 0; i < count + 1; i++) {
        stack_pop(stack);
    }

    stack_push(stack, NON_TERM);
    return EC_SUCCESS;
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

    ret_val = get_token(new_token);
    if (ret_val) {
        return ret_val;
    }

    do {
        top_term = stack_top_term(&stack_prec);
        symbol = token_to_symbol(new_token);

        // get precedence symbol from precedence table
        prec_symbol = prec_table[symbol_to_index(top_term->data)][symbol_to_index(symbol)];
        switch (prec_symbol) {
            case '=':
                stack_push(&stack_prec, symbol);
                GET_NEW_TOKEN(new_token, ret_val);
                break;

            case '<':
                stack_push_above_term(&stack_prec, HANDLE);
                stack_push(&stack_prec, symbol);

                GET_NEW_TOKEN(new_token, ret_val);
                break;
            case '>':
                if (symbol == ID && top_term->data == ID) {
                    // loaded two IDs, possible multiple assignmemts on one line
                    // continue reducing
                    symbol = DOLLAR;
                }
                // reduce
                ret_val = reduce(&stack_prec);
                if (ret_val) {
                    return ret_val;
                }
                break;

            default:
                if (symbol == DOLLAR && top_term->data != DOLLAR) {
                    end = 1;
                }
                break;
        }

    } while (!end);

    if (!(stack_prec.top->data == NON_TERM && stack_prec.top->next->data == DOLLAR)) {
        // final state of stack is not $E
        return ret_val = ERROR_SYNTAX;
    }

    return_token = new_token;
    return ret_val;
}
