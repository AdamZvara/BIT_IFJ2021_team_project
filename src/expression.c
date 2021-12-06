/**
 * VUT IFJ Project 2021.
 *
 * @file expression.c
 *
 * @brief Implementation of bottom up parsing of expressions
 *
 * @author Vojtěch Eichler
 * @author Václav Korvas
 * @author Tomáš Matuš
 * @author Adam Zvara
 */

#include <stdio.h>
#include <stdlib.h>

#include "stack.h"
#include "expression.h"
#include "scanner.h"
#include "symtable.h"
#include "error.h"
#include "parser.h"
#include "generator.h"

const char prec_table[TABLE_SIZE][TABLE_SIZE] = {
    // ---> current token
    //| # |/*//| +- | .. |  r | (  |  ) |  id | $                | what is on stack_top
    { '-', '>', '>', '-' , '>', '<', '>', '<', '>' }, // #       |
    { '<', '>', '>', '-' , '>', '<', '>', '<', '>' }, // /*//    v
    { '<', '<', '>', '-' , '>', '<', '>', '<', '>' }, // +-
    { '-', '-', '-', '<' , '>', '<', '>', '<', '>' }, // ..
    { '<', '<', '<', '<' , '-', '<', '>', '<', '>' }, // r ( < <= > >= )
    { '<', '<', '<', '<' , '<', '<', '=', '<', '-' }, // (
    { '>', '>', '>', '>' , '>', '-', '>', '>', '>' }, // )
    { '-', '>', '>', '>' , '>', '-', '>', '>', '>' }, // id
    { '<', '<', '<', '<' , '<', '<', '-', '<', '-' }, // $
};


/**
 * @brief Converts token to a symbol
 */
int token_to_symbol(token_t *token)
{
    token_type_t type = token->type;
    switch (type) {
        case TOK_LEN:
            return STR_LEN;
        case TOK_MUL:
            return MUL;
        case TOK_DIV:
            return DIV;
        case TOK_INT_DIV:
            return DIV_INT;
        case TOK_PLUS:
            return PLUS;
        case TOK_MINUS:
            return MINUS;
        case TOK_CONCAT:
            return CONCAT;
        case TOK_EQ:
            return EQ;
        case TOK_NEQ:
            return NOT_EQ;
        case TOK_LES:
            return LESS;
        case TOK_LES_EQ:
            return LESS_EQ;
        case TOK_GR:
            return GREAT;
        case TOK_GR_EQ:
            return GREAT_EQ;
        case TOK_LBRACKET:
            return LEFT_BR;
        case TOK_RBRACKET:
            return RIGHT_BR;
        case TOK_ID:
            return ID;
        case TOK_INT:
            return INT;
        case TOK_DECIMAL:
            return NUM;
        case TOK_STRING:
            return STR;
        case TOK_KEYWORD:
            if (token->attribute.keyword == KW_NIL) {
                return NIL;
            }
            return DOLLAR;

        default:
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

        case CONCAT:
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
        case INT:
        case NUM:
        case STR:
        case NIL:
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
        if (!(top->data >= ID && top->data <= NIL)) {
            return ERROR_SYNTAX;
        }
    } else if (count == 2 ) {
        if (top->data == NON_TERM && top->next->data == STR_LEN){
            // unary operator #
            generate_push_operator(top->next->data);
        } else {
            return ERROR_SYNTAX;
        }
    } else if (count == 3 ) {
        if (top->data == NON_TERM && top->next->next->data == NON_TERM) {
            if (top->next->data >= MUL && top->next->data <= GREAT_EQ) {
                // binary operators
                generate_push_operator(top->next->data);
            }
        } else if (top->data == RIGHT_BR && top->next->next->data == LEFT_BR) {
            if (!(top->next->data == NON_TERM)) {
                return ERROR_SYNTAX;
            }
        } else {
            return ERROR_SYNTAX;
        }
    } else {
        // incorrect count
        return ERROR_SYNTAX;
    }

    // replace rule with NON_TERM
    for (int i = 0; i < count + 1; i++) {
        stack_pop(stack);
    }

    stack_push(stack, NON_TERM);
    return SUCCESS;
}

int check_semantic(token_t *token, stack_t *stack, int *type)
{
    struct local_data *check_id;
    stack_item_t *top;

    switch (token->type){
        case TOK_INT:
            if (*type == T_NONE) {
                *type = T_INT;
                break;
            }
            top = get_top_operator(stack);
            if (top->data == STR_LEN || top->data == CONCAT) {
                return ERROR_SEMANTIC_TYPE;
            } else if (!(*type == T_INT || *type == T_NUM)) {
                return ERROR_SEMANTIC_TYPE;
            }
            break;

        case TOK_DECIMAL:
            if (*type == T_NUM || *type == T_NONE) {
                *type = T_NUM;
            } else if (*type ==T_INT) {
                generate_int_to_num();
                *type = T_NUM;
            }
            top = get_top_operator(stack);
            if (top->data == STR_LEN || top->data == CONCAT) {
                return ERROR_SEMANTIC_TYPE;
            } else if (!(*type == T_NUM)) {
                return ERROR_SEMANTIC_TYPE;
            }
            break;

        case TOK_STRING:
            if (*type == T_NONE) {
                *type = T_STR;
                break;
            }
            top = get_top_operator(stack);
            if (top->data >= MUL && top->data <= MINUS) {
                return ERROR_SEMANTIC_TYPE;
            } else if (*type != T_STR) {
                if (!find_len_op(stack)) {
                    return ERROR_SEMANTIC_TYPE;
                }
            }
            break;

        case TOK_ID:
            // found identificator, check if it's declared
            top = stack_top(stack);
            // skip if ID ID
            if (!((top->data >= ID && top->data <= STR) || top->data == RIGHT_BR || top->data == NON_TERM)) {
                check_id = local_find(local_tab, token->attribute.s);
                if(check_id) {
                    /* TODO
                    if (check_id->init == false) {
                        return ERROR_SEMANTIC;
                    }*/

                    if (*type == T_NONE) {
                        if (check_id->type == INT_T) {
                            *type = T_INT;
                        } else if (check_id->type == NUM_T) {
                            *type = T_NUM;
                        } else if (check_id->type == STR_T) {
                            *type = T_STR;
                        } else if (check_id->type == NIL_T) {
                            *type = T_NIL;
                        }
                        break;
                    }
                    top = get_top_operator(stack);
                    if (check_id->type == INT_T) {
                        if (top->data == STR_LEN || top->data == CONCAT) {
                            return ERROR_SEMANTIC_TYPE;
                        } else if (!(*type == T_INT || *type == T_NUM)) {
                            return ERROR_SEMANTIC_TYPE;
                        }
                    } else if (check_id->type == NUM_T) {
                        if (*type == T_INT) {
                            generate_int_to_num();
                            *type = T_NUM;
                        }
                        if (top->data == STR_LEN || top->data == CONCAT) {
                            return ERROR_SEMANTIC_TYPE;
                        } else if (!(*type == T_NUM)) {
                            return ERROR_SEMANTIC_TYPE;
                        }
                    } else if (check_id->type == STR_T) {
                        if (top->data >= MUL && top->data <= MINUS) {
                            return ERROR_SEMANTIC_TYPE;
                        } else if (*type != T_STR) {
                            if (!find_len_op(stack)) {
                                return ERROR_SEMANTIC_TYPE;
                            }
                        }
                    }
                } else if (!global_find(global_tab, token->attribute.s)) {
                    // ID is not a function => variable doesn't exist
                    return ERROR_SEMANTIC;
                }
            }
            break;

        case TOK_KEYWORD:
            if (token->attribute.keyword == KW_NIL) {
                if (*type == T_NONE) {
                    *type = T_NIL;
                    break;
                }
                top = stack_top(stack);
                if (!(top->data == DOLLAR || (top->data >= EQ && top->data <= GREAT_EQ))) {
                    return ERROR_NIL;
                }
                break;
            }
            break;

        case TOK_PLUS:
        case TOK_MINUS:
        case TOK_MUL:
            top = stack_top(stack);
            if ((top->data == STR && !find_len_op(stack)) || (top->data == NON_TERM && *type == T_STR)) {
                return ERROR_SEMANTIC_TYPE;
            } else if (top->data == NIL) {
                return ERROR_NIL;
            }
            break;

        case TOK_DIV:
            // div always yields 'number' result
            top = stack_top(stack);
            if ((top->data == STR && find_len_op(stack)) || (top->data == NON_TERM && *type == T_STR)) {
                return ERROR_SEMANTIC_TYPE;
            } else if (top->data == NIL) {
                return ERROR_NIL;
            }
            *type = T_NUM;
            break;

        case TOK_CONCAT:
            top = stack_top(stack);
            if (top->data == INT || top->data == NUM ||
                    (top->data == NON_TERM && *type != T_STR && !find_len_op(stack))) {
                return ERROR_SEMANTIC_TYPE;
            } else if (top->data == NIL) {
                return ERROR_NIL;
            }

            break;

        case TOK_LEN:
            if (*type == T_NONE) {
                *type = T_INT;
            } else if (*type == T_STR) {
                return ERROR_SEMANTIC_TYPE;
            }

        default:
            break;

    // aritmetic incompatibility ERROR_SEMANTIC_TYPE
    // ID is defined ERROR_SEMANTIC
    }

    return SUCCESS;
}

void push_operand(token_t *token, int *type)
{
    struct local_data *id = NULL;

    generate_push_operand(token);

    if (token->type == TOK_INT && *type == T_NUM) {
        // convert current token to number
        generate_int_to_num();
    } else if (token->type == TOK_ID) {
        id = local_find(local_tab, token->attribute.s);
        if (id->type == INT_T && *type == T_NUM) {
            generate_int_to_num();
        }
    }
}

int expression(token_t **return_token)
{
    int end = 0;
    int ret_val = SUCCESS;
    int expr_type = T_NONE;

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

    top_term = stack_top_term(&stack_prec);
    symbol = token_to_symbol(new_token);

    while (!end) {
        ret_val = check_semantic(new_token, &stack_prec, &expr_type);
        if (ret_val) {
            EXIT_ON_ERROR(ret_val);
        }

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

                // push operand
                if (new_token->type == TOK_ID ||
                        (new_token->type == TOK_KEYWORD && new_token->attribute.keyword == KW_NIL) ||
                        (new_token->type >= TOK_INT && new_token->type <= TOK_STRING)) {

                    if (new_token->type == TOK_ID && global_find(global_tab, new_token->attribute.s)) {
                        // ID is a function
                        *return_token = new_token;
                        stack_dispose(&stack_prec);
                        return EC_FUNC;
                    }
                    push_operand(new_token, &expr_type);
                }

                GET_NEW_TOKEN(new_token, ret_val);
                break;

            case '>':
                if ((symbol >= ID && symbol <= STR) &&
                        ((top_term->data >= ID && top_term->data <= STR) || top_term->data == RIGHT_BR)) {
                    // loaded two IDs, possible multiple assignmemts on one line
                    // continue reducing
                    symbol = DOLLAR;
                }
                // reduce
                ret_val = reduce(&stack_prec);
                if (ret_val) {
                    // couldn't find rule to reduce
                    EXIT_ON_ERROR(ret_val);
                }
                break;

            default:
                end = 1;
                break;
        }

        if (symbol != DOLLAR) {
            symbol = token_to_symbol(new_token);
        }
        top_term = stack_top_term(&stack_prec);

    } // end while

    if (!(stack_prec.top->data == NON_TERM && stack_prec.top->next->data == DOLLAR)) {
        // final state of stack is not $E
        free(new_token);
        ret_val = ERROR_SYNTAX;
        *return_token = NULL;
    } else {
        *return_token = new_token;
        ret_val = expr_type;
    }

    stack_dispose(&stack_prec);
    return ret_val;
}

#ifdef EXPR_TEST
int main(){
    token_t **returned = NULL;
    int rv = expression(returned);
    return rv;
}
#endif
