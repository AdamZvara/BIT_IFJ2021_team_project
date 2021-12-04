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
#include "generator.h"
#include "ibuffer.h"
#include "builtin.h"
#include "expression.h"
#include "parser_helper.h"


token_t *curr_token = NULL;
token_t *backup_token = NULL;
token_t *ret_token = NULL;

global_symtab_t *global_tab = NULL;
local_symtab_t *local_tab = NULL;

ibuffer_t *buffer = NULL;
ibuffer_t *defvar_buffer = NULL;
builtin_used_t *builtin_used = NULL;

parser_helper_t *p_helper = NULL;

int ret = SUCCESS;

void token_free()
{
    FREE_TOK_STRING();
    free(curr_token);
}

int parse()
{
    curr_token = malloc(sizeof(token_t));
    if (!curr_token) {
        return ERROR_INTERNAL;
    }

    // create global symtable
    global_tab = global_create();
    if (global_tab == NULL) {
        return ERROR_INTERNAL;
    }

    // add builtin function to global symtable
    add_builtin(global_tab);

    // structure to indicate which builtins have been used
    builtin_used = builtin_used_create();
    if (builtin_used == NULL) {
        return ERROR_INTERNAL;
    }

    // create ibuffer to store generated instructions
    buffer = ibuffer_create(IBUFFER_SIZE, INSTR_SIZE);
    if (buffer == NULL) {
        return ERROR_INTERNAL;
    }

    // create ibuffer for defvar instruction inside while statement
    defvar_buffer = ibuffer_create(IBUFFER_SIZE, INSTR_SIZE);
    if (defvar_buffer == NULL) {
        return ERROR_INTERNAL;
    }

    // create parser helper
    p_helper = p_helper_create();
    if (p_helper == NULL) {
        return ERROR_INTERNAL;
    }

    ret = require();

    // check if all functions were defined - ret has higher priority
    if (!ret && global_check_declared(global_tab)) {
        return ERROR_SEMANTIC;
    }

    ibuffer_print(buffer);
    ibuffer_destroy(buffer);
    global_destroy(global_tab);
    if (curr_token != NULL) {
        token_free();
    }
	return ret;
}

int require()
{
    // read new token, should be require keyword, also check for failure
    ret = get_token(curr_token);
    if (ret)
        return ret;
    if ((GET_TYPE != TOK_KEYWORD) || (GET_KW != KW_REQUIRE))
        return ERROR_SYNTAX;

    // check for string after _require_ keyword
    FREE_TOK_STRING();
    ret = get_token(curr_token);
    if (ret)
        return ret;
    if (GET_TYPE != (token_type_t)TOK_STRING)
        return ERROR_SYNTAX;

    // generate starting instruction
    generate_start();

    // go to rule <prog>
    return prog();
}

int prog()
{
    // initialize helper structure to parser function dec/def
    p_helper_clear(p_helper);

    // entry point has been generated
    static bool entry = false;

    if (!backup_token) {
        NEXT_TOKEN();
    } else {
        curr_token = backup_token;
        backup_token = NULL;
    }
    if (GET_TYPE == TOK_KEYWORD) { // new token is keyword
        if (GET_KW == KW_GLOBAL) { // check if keyword is _global_
            // get new token that should be ID

            NEXT_TOKEN();
            if (GET_TYPE != TOK_ID)
                return ERROR_SYNTAX;

            // check if function is in global symtable
            p_helper->func = global_find(global_tab, GET_ID);
            if (p_helper->func != NULL) {
                // multiple declarations of function
                return ERROR_SEMANTIC;
            }

            // add function to global symtable
            if ((p_helper->func = global_add(global_tab, GET_ID)) == NULL) {
                return ERROR_INTERNAL;
            }

            // get new token that should be colon
            NEXT_TOKEN();
            if (GET_TYPE != TOK_COLON)
                return ERROR_SYNTAX;

            // get new token that should be keyword _function_
            NEXT_TOKEN();
            if (GET_TYPE != TOK_KEYWORD || GET_KW != KW_FUNCTION)
                return ERROR_SYNTAX;

            // get new token that should be opening bracket
            NEXT_TOKEN();
            if (GET_TYPE != TOK_LBRACKET)
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

        } else if (GET_KW == KW_FUNCTION) { // check if keyword is _function_
            // get new token that should be ID
            NEXT_TOKEN();
            if (GET_TYPE != TOK_ID)
                return ERROR_SYNTAX;

            // check if function is already in global symtable
            p_helper->func = global_find(global_tab, GET_ID);
            if (p_helper->func == NULL) {
                // create new record in global symtable
                if ((p_helper->func = global_add(global_tab, GET_ID)) == NULL) {
                    return ERROR_INTERNAL;
                }
                // function was not found in global symtable
                p_helper->func->defined = true;
            } else {
                // function was found, check if it is already defined
                if (p_helper->func->defined) {
                    // function is in global symtable -> redefinition
                    return ERROR_SEMANTIC;
                }
                p_helper->func->defined = true;
                p_helper->func_found = true;
            }

            // create local symtable for function
            if ((local_tab = local_create(GET_ID)) == NULL) {
                return ERROR_INTERNAL;
            }

            // get new token that should be opening bracket
            NEXT_TOKEN();
            if (GET_TYPE != TOK_LBRACKET)
                return ERROR_SYNTAX;

            ret = params_2();
            if (ret)
                return ret;

            // clear string containing temporary params/retvals
            p_helper_clear_string(p_helper);

            // step into <ret_params> rule
            ret = ret_params();
            if (ret)
                return ret;

            // generate function label + retvals and parameters
            generate_function(p_helper);

            ret = body();
            if (ret)
                 return ret;

            return prog();

        } else { // unexpected keyword, return error
            return ERROR_SYNTAX;
        }
    } else if (GET_TYPE == TOK_ID) { // new token is ID = function call
        // try to find function in global symtable
        p_helper->func = global_find(global_tab, GET_ID);
        // if function was not found or was not defined
        if (p_helper->func == NULL) {
            return ERROR_SEMANTIC;
        }

        builtin_used_update(builtin_used, p_helper->func->key);

        NEXT_TOKEN();
        if (GET_TYPE != TOK_LBRACKET)
            return ERROR_SYNTAX;

        // no entry label was created, create one
        if (!entry) {
            generate_entry();
            entry = true;
        }

        // dont create new frame if function is write
        if (strcmp(p_helper->func->key.str, "write")) {
            generate_call_prep(p_helper);
        }

        // <args>
        ret = args();
        if (ret)
            return ret;

        return prog();

    }  else if (GET_TYPE == TOK_EOF) {

        // generate end label to skip builtin functions
        generate_end();

        // generate used builtin functions
        generate_builtin(builtin_used);

        // generate division by zero exit
        generate_div_by_zero();

        // generate nil with any operation
        generate_nil_with_operator();

        // generate write nil
        generate_write_nil();

        // generate end label
        generate_exit();

        return ret;
    } else { // unexpected token, return error
        return ERROR_SYNTAX;
    }
}

int params()
{
    NEXT_TOKEN();
    if (GET_TYPE == TOK_RBRACKET) {
        return ret;
    }

    if ((GET_TYPE == TOK_KEYWORD && GET_KW == KW_STRING) ||
        (GET_TYPE == TOK_KEYWORD && GET_KW == KW_NUMBER) ||
        (GET_TYPE == TOK_KEYWORD && GET_KW == KW_INTEGER)) {
        p_helper_set_params(p_helper, GET_KW);
        return params_n();
    } else {
        return ERROR_SYNTAX;
    }
}

int params_n()
{
    NEXT_TOKEN();
    if (GET_TYPE == TOK_RBRACKET) {
        return ret;
    } else if (GET_TYPE == TOK_COMMA) {
        NEXT_TOKEN();
        if ((GET_TYPE == TOK_KEYWORD && GET_KW == KW_STRING) ||
            (GET_TYPE == TOK_KEYWORD && GET_KW == KW_NUMBER) ||
            (GET_TYPE == TOK_KEYWORD && GET_KW == KW_INTEGER)) {
            p_helper_set_params(p_helper, GET_KW);
            return params_n();
        }  else {
            return ERROR_SYNTAX;
        }
    } else {
        return ERROR_SYNTAX;
    }
}

int params_2()
{
    // first check if params are empty or not
    NEXT_TOKEN();
    if (GET_TYPE == TOK_RBRACKET) {
        if (p_helper->func_found) {
            if (!str_empty(p_helper->func->params)) {
                // params are empty but helper temp is not
                return ERROR_SEMANTIC;
            }
        }
        return ret;

    } else if (GET_TYPE == TOK_ID) { // params start correctly with ID
        // check if there is a function with the same name as variable
        if (global_find(global_tab, GET_ID))
            return ERROR_SEMANTIC;

        // add identifier to local symtable
        //p_helper->id = local_add(local_tab, GET_ID, true);
        p_helper_add_identifier(p_helper, local_add(local_tab, GET_ID, true));

        // now check the rest of the syntax and go to params_2_n if everything is correct
        NEXT_TOKEN();
        if (GET_TYPE != TOK_COLON)
            return ERROR_SYNTAX;
        // next few lines of code check <types_keyword> rule
        NEXT_TOKEN();

        if ((GET_TYPE == TOK_KEYWORD && GET_KW == KW_STRING) ||
            (GET_TYPE == TOK_KEYWORD && GET_KW == KW_NUMBER) ||
            (GET_TYPE == TOK_KEYWORD && GET_KW == KW_INTEGER)) {
            // add parameters to global symtable
            p_helper_set_params(p_helper, GET_KW);
            local_add_type(p_helper->id_first->data, GET_KW);
            return params_2_n();
        } else {
            return ERROR_SYNTAX;
        }
    } else {
        return ERROR_SYNTAX;
    }
}

int params_2_n()
{
    // check for end of params
    NEXT_TOKEN();
    if (GET_TYPE == TOK_RBRACKET) {
        if (p_helper->func_found) {
            // function is in global table, check if parameters match
            if (!str_isequal(p_helper->func->params, p_helper->temp)) {
                return ERROR_SEMANTIC;
            }
        }
        return ret;

    } else if (GET_TYPE == TOK_COMMA) { // check for comma
        // check the rest of the syntax and descend into parama_2_n if everything is correct
        NEXT_TOKEN();
        if (GET_TYPE != TOK_ID)
            return ERROR_SYNTAX;

        // check if there is a function of the same name as variable
        if (global_find(global_tab, GET_ID))
            return ERROR_SEMANTIC;

        // add identifier to local symtable
        //p_helper->id = local_add(local_tab, GET_ID, true);
        p_helper_add_identifier(p_helper, local_add(local_tab, GET_ID, true));

        NEXT_TOKEN();
        if (GET_TYPE != TOK_COLON)
            return ERROR_SYNTAX;

        // next lines check <types_keyword> rule
        NEXT_TOKEN();
        if ((GET_TYPE == TOK_KEYWORD && GET_KW == KW_STRING) ||
            (GET_TYPE == TOK_KEYWORD && GET_KW == KW_NUMBER) ||
            (GET_TYPE == TOK_KEYWORD && GET_KW == KW_INTEGER)) {
            // add parameters to global symtable
            p_helper_set_params(p_helper, GET_KW);
            local_add_type(p_helper->id_last->data, GET_KW);
            return params_2_n();
        } else {
            return ERROR_SYNTAX;
        }
    } else {
        return ERROR_SYNTAX;
    }
}

int ret_params()
{
    NEXT_TOKEN();
    if (GET_TYPE != TOK_COLON) {
        // function is in global table, check if retvals match
        // current token is COLON so retvals should be empty
        if (p_helper->func_found) {
            if (str_empty(p_helper->func->retvals)) {
                backup_token = curr_token;
                return ret;
            }
            return ERROR_SEMANTIC;
        }
        backup_token = curr_token;
        return ret;
    }
    NEXT_TOKEN();

    if ((GET_TYPE == TOK_KEYWORD && GET_KW == KW_STRING) ||
        (GET_TYPE == TOK_KEYWORD && GET_KW == KW_NUMBER) ||
        (GET_TYPE == TOK_KEYWORD && GET_KW == KW_INTEGER)) {
        p_helper_set_retvals(p_helper, GET_KW);
        return ret_params_n();
    }  else {
        return ERROR_SYNTAX;
    }
}

int ret_params_n()
{
    NEXT_TOKEN();
    if (GET_TYPE != TOK_COMMA) {
        // function is in global table, check if retvals match
        // retvals are not empty, compare them with p_helper temporary string
        if (p_helper->func_found) {
            if (!str_isequal(p_helper->func->retvals, p_helper->temp)) {
                return ERROR_SEMANTIC;
            }
        }
        backup_token = curr_token;
        return ret;
    }
    NEXT_TOKEN();
    if ((GET_TYPE == TOK_KEYWORD && GET_KW == KW_STRING) ||
        (GET_TYPE == TOK_KEYWORD && GET_KW == KW_NUMBER) ||
        (GET_TYPE == TOK_KEYWORD && GET_KW == KW_INTEGER)) {
        p_helper_set_retvals(p_helper, GET_KW);
        return ret_params_n();
    } else {
        return ERROR_SYNTAX;
    }
}

int body()
{
    if (!backup_token) {
        NEXT_TOKEN();
    } else {
        curr_token = backup_token;
        backup_token = NULL;
    }

    // clear helper structure
    p_helper_clear(p_helper);

    // print out instruction buffer
    if (!str_contains(p_helper->status, 'w')) {
        ibuffer_print(defvar_buffer);
        ibuffer_clear(defvar_buffer);
        ibuffer_print(buffer);
        ibuffer_clear(buffer);
    }

    if (GET_TYPE == TOK_KEYWORD) {
        switch (GET_KW) {
            case KW_LOCAL:  // Declaration of local variable
                NEXT_TOKEN();
                if (GET_TYPE != TOK_ID)
                    return ERROR_SYNTAX;

                // if variable was defined in this block, return error
                int index = hash_function(GET_ID);
                struct local_data *current = local_tab->data[index];
                while(current) {
                    if (str_isequal(current->name, GET_ID))
                        return ERROR_SEMANTIC;
                    current = current->next;
                }

                // if variable has same name as function
                if (global_find(global_tab, GET_ID))
                    return ERROR_SEMANTIC;

                // add identifer to local symtable
                p_helper_add_identifier(p_helper, local_add(local_tab, GET_ID, false));

                // if we are in while statement, append to defvar buffer
                if (str_contains(p_helper->status, 'w')) {
                    generate_identifier(defvar_buffer, GET_ID);
                } else {
                    generate_identifier(buffer, GET_ID);
                }

                NEXT_TOKEN();
                if (GET_TYPE != TOK_COLON)
                    return ERROR_SYNTAX;
                NEXT_TOKEN();
                if ((GET_TYPE == TOK_KEYWORD && GET_KW == KW_STRING) ||
                   (GET_TYPE == TOK_KEYWORD && GET_KW == KW_NUMBER) ||
                   (GET_TYPE == TOK_KEYWORD && GET_KW == KW_INTEGER)) {
                    local_add_type(p_helper->id_first->data, GET_KW);
                    ret = init();
                    if (ret)
                        return ret;
                }
                return body();
                break;
            case KW_IF: // IF <expr> THEN <body> ELSE <body> END <body>
                // add if in previous tab, because name is created from if_counter in it
                local_add_if(local_tab);
                // add new depth so local variables can be recognized
                local_new_depth(&local_tab);

                str_add_char(&p_helper->status, 'i');

                // call expression()
                ret = expression(&backup_token);
                FREE_TOK_STRING();
                free(curr_token);
                curr_token = backup_token;
                if (ret == EC_FUNC) {
                    return ERROR_SYNTAX;
                } else if (ret >= T_INT && ret <= T_NIL) {
                    // success
                    ret = 0;
                } else {
                    return ret;
                }

                generate_if_else();

                // THEN
                if (GET_TYPE != TOK_KEYWORD || GET_KW != KW_THEN)
                    return ERROR_SYNTAX;

                backup_token = NULL;

                // <body>
                ret = body();
                if (ret)
                    return ret;

                // ELSE is checked by the body call above

                // Delete if then scope and add new scope for else branch
                local_delete_top(&local_tab);
                local_new_depth(&local_tab);
                // Update local if counter
                local_add_if(local_tab);
                local_after_else(local_tab);

                // <body>
                ret = body();
                if (ret)
                    return ret;

                // END is checked by the body call above

                // delete top symtable
                local_delete_top(&local_tab);

                // <body>
                return body();
                break;
            case KW_WHILE:
                // update while counter
                local_add_while(local_tab);
                // add new depth so local variables can be recognized
                local_new_depth(&local_tab);

                str_add_char(&p_helper->status, 'w');

                generate_while_start();

                // call expr()
                ret = expression(&backup_token);
                FREE_TOK_STRING();
                free(curr_token);
                curr_token = backup_token;
                if (ret == EC_FUNC) {
                    return ERROR_SYNTAX;
                } else if (ret >= T_INT && ret <= T_NIL) {
                    // success
                    ret = 0;
                } else {
                    return ret;
                }

                generate_while_skip();

                // DO - already read by expression()
                if (GET_TYPE != TOK_KEYWORD || GET_KW != KW_DO)
                    return ERROR_SYNTAX;

                backup_token = NULL;

                // <body>
                ret = body();
                if (ret)
                    return ret;

                // END is checked by the body call above

                // delete top symtable
                local_delete_top(&local_tab);

                // <body>
                return body();
                break;
            case KW_END:
                if (local_tab->depth == 0) {
                    // generate return code
                    generate_function_end();
                    // destroy local symtable for function
                    local_destroy(local_tab);
                    local_tab = NULL;
                } else {
                    if (str_getlast(p_helper->status) == 'i') {
                        generate_if_end();
                    } else if (str_getlast(p_helper->status) == 'w') {
                        generate_while_end();
                    }
                    str_clearlast(&p_helper->status);
                }
                return ret;
                break;
            case KW_ELSE:
                generate_else();
                return ret;
                break;
            case KW_RETURN:
                ret = r_side();
                if (ret)
                    return ret;

                struct global_item *tmp = global_find(global_tab, local_tab->key);
                if ((int)tmp->retvals.length < p_helper->par_counter)
                        return ERROR_SEMANTIC_PARAMS;

                generate_function_end();

                return body();
                break;
            default:
                return ERROR_SYNTAX;
                break;
        }
    } else if (GET_TYPE == TOK_ID) { // ID <body_n> <body>
        // in case of function call
        p_helper->func = global_find(global_tab, GET_ID);

        // in case of ID assign
        //p_helper->id = local_find(local_tab, GET_ID);
        p_helper_add_identifier(p_helper, local_find(local_tab, GET_ID));

        ret = body_n();
        if (ret)
            return ret;

        return body();
    } else {
        return ERROR_SYNTAX;
    }
}

int body_n()
{
    if (backup_token) {
        curr_token = backup_token;
        backup_token = NULL;
    } else {
        NEXT_TOKEN();
    }

    if (GET_TYPE == TOK_LBRACKET) {
        // function is not found
        if (p_helper->func == NULL) {
            return ERROR_SEMANTIC;
        }

        builtin_used_update(builtin_used, p_helper->func->key);

        // dont create new frame if function is write
        if (strcmp(p_helper->func->key.str, "write")) {
            generate_call_prep(p_helper);
        }

        return args(p_helper);
    } else if (GET_TYPE == TOK_ASSIGN) {
        // check if variable was defined
        if (p_helper->id_first->data == NULL) {
            return ERROR_SEMANTIC;
        }
        p_helper->assign = true;
        return assign_single(p_helper);
    } else if (GET_TYPE == TOK_COMMA) {
        NEXT_TOKEN();
        if (GET_TYPE != TOK_ID)
            return ERROR_SYNTAX;

        // check if variable was defined
        if (p_helper->id_first->data == NULL) {
            return ERROR_SEMANTIC;
        }

        p_helper->assign = true;

        // add loaded identifier into p_helper structure
        p_helper_add_identifier(p_helper, local_find(local_tab, GET_ID));

        // check if other variable was defined
        if (p_helper->id_last->data == NULL) {
            return ERROR_SEMANTIC;
        }

        ret = assign_multi();
        if (ret)
            return ret;

        ret = r_side();
        if (ret)
            return ret;

        //ibuffer_revert_expression(buffer);

        return ret;
    } else {
        return ERROR_SYNTAX;
    }
}

int assign_single()
{
    // call expression()
    ret = expression(&backup_token);
    if (ret >= T_INT && ret <= T_NIL) {
        // success
        ret = 0;
        generate_assign(p_helper->id_first->data->name);
        FREE_TOK_STRING();
        free(curr_token);
        curr_token = backup_token;
        return ret;

    } else if (ret == EC_FUNC) {
        // free curr_token and use token given by expression instead
        FREE_TOK_STRING();
        free(curr_token);
        curr_token = backup_token;
        backup_token = NULL;

        // perform function call
        p_helper->func = global_find(global_tab, GET_ID);

        ret = body_n();

        return ret;
    }
    else
        return ret;
}

int assign_multi()
{
    NEXT_TOKEN();
    if (GET_TYPE == TOK_COMMA) {
        NEXT_TOKEN();
        if (GET_TYPE != TOK_ID)
            return ERROR_SYNTAX;
        p_helper_add_identifier(p_helper, local_find(local_tab, GET_ID));
        // check if last added variable was defined
        if (p_helper->id_last->data == NULL) {
            return ERROR_SEMANTIC;
        }
        return assign_multi();
    } else if (GET_TYPE == TOK_ASSIGN) {
        return ret;
    } else {
        return ERROR_SYNTAX;
    }
}

int r_side()
{
    // call expression()
    ret = expression(&backup_token);
    if (ret >= T_INT && ret <= T_NIL) {
        // success
        ret = 0;
        if (p_helper->assign) {
            // assign value to variable
            generate_assign(p_helper->id_first->data->name);
            p_helper_delete_identifier(p_helper);
        } else {
            generate_return_value(p_helper->par_counter);
            p_helper->par_counter++;
        }
        FREE_TOK_STRING();
        free(curr_token);
        curr_token = backup_token;
        return r_side_n();
    } else if (ret == EC_FUNC) {
        // free curr_token and use token given by expression instead
        FREE_TOK_STRING();
        free(curr_token);
        curr_token = backup_token;
        backup_token = NULL;

        // perform function call
        p_helper->func = global_find(global_tab, GET_ID);

        builtin_used_update(builtin_used, p_helper->func->key);

        ret = body_n();
        if (ret)
            return ret;

        return r_side_n();
    }
    else
        return ret;
}

int r_side_n()
{
    if (backup_token) {
        curr_token = backup_token;
        backup_token = NULL;
    } else
        NEXT_TOKEN();

    if (GET_TYPE == TOK_COMMA) {
        return r_side();
    } else {
        backup_token = curr_token;
        return ret;
    }
}

int init()
{
    NEXT_TOKEN();
    if (GET_TYPE == TOK_ASSIGN) {
        p_helper->id_first->data->init = true;
        return init_n();
    } else {
        backup_token = curr_token;
        return ret;
    }
}

int init_n()
{
    // call expression()
    ret = expression(&backup_token);
    if (ret >= T_INT && ret <= T_NIL) {
        // success
        ret = 0;
        generate_assign(p_helper->id_first->data->name);
        FREE_TOK_STRING();
        free(curr_token);
        curr_token = backup_token;
        return ret;
    } else if (ret == EC_FUNC) {
        // free curr_token and use token given by expression instead
        FREE_TOK_STRING();
        free(curr_token);
        curr_token = backup_token;
        backup_token = NULL;

        p_helper->func = global_find(global_tab, GET_ID);
        p_helper->assign = true;

        // check if function returns any value
        if (str_len(p_helper->func->retvals) != 0) {
            switch (p_helper->id_first->data->type)
            {
            case STR_T:
                if (p_helper->func->retvals.str[0] != 's')
                    return ERROR_SEMANTIC_ASSIGN;
                break;

            case INT_T:
                if (p_helper->func->retvals.str[0] != 'i')
                    return ERROR_SEMANTIC_ASSIGN;
                break;

            case NUM_T:
                if ((p_helper->func->retvals.str[0] != 'n') && (p_helper->func->retvals.str[0] != 'i')) {
                    return ERROR_SEMANTIC_ASSIGN;
                }
                break;

            default:
                break;
            }
        } else {
            return ERROR_SEMANTIC_PARAMS;
        }

        builtin_used_update(builtin_used, p_helper->func->key);

        ret = body_n();

        return ret;
    } else
        return ret;
}

int args()
{
    NEXT_TOKEN();
    if (GET_TYPE == TOK_RBRACKET) {
        if (!strcmp(p_helper->func->key.str, "write")) {
            // special case for write functions - variadic functions
            // parameters are not checked
            return ret;
        }

        //if (!str_isequal(p_helper->func->params, p_helper->temp)) {
        //        // function call does not match with definition params
        //        return ERROR_SEMANTIC_PARAMS;
        //    }
        if (p_helper->func->params.length != p_helper->temp.length)
                return ERROR_SEMANTIC_PARAMS;
        for (unsigned i = 0; i < p_helper->temp.length; i++) {
            if ((p_helper->func->params.str[i] != p_helper->temp.str[i]) &&
               ((p_helper->temp.str[i] == 'i') && (p_helper->func->params.str[i] == 'n'))) {
                generate_num_conversion(i);
            } else if ((p_helper->func->params.str[i] != p_helper->temp.str[i])) {
                return ERROR_SEMANTIC_PARAMS;
            }
        }
        generate_call(p_helper);

        if (p_helper->id_first != NULL) {
            if (p_helper->id_first->data != NULL && p_helper->assign) {
                // assign function return value into variable(s)
                generate_assign_function(p_helper);
            }
        }
        return ret;
    }

    if (GET_TYPE == TOK_STRING || GET_TYPE == TOK_DECIMAL || GET_TYPE == TOK_INT) {
        if (!strcmp(p_helper->func->key.str, "write")) {
            // instructions are generated on spot
            generate_write(curr_token);
            return args_n();
        }
        p_helper_call_params_const(p_helper, GET_TYPE);
        generate_call_params(curr_token, p_helper);
        return args_n();
    } else if (GET_TYPE == TOK_ID) {
        if (local_find(local_tab, GET_ID) == NULL) {
            return ERROR_SEMANTIC;
        }

        if (!strcmp(p_helper->func->key.str, "write")) {
            // instructions are generated on spot
            generate_write(curr_token);
            return args_n();
        }
        p_helper_call_params_id(p_helper, GET_ID);
        generate_call_params(curr_token, p_helper);
        return args_n();
    } else {
        return ERROR_SYNTAX;
    }
}

int args_n() {
    NEXT_TOKEN();
    if (GET_TYPE == TOK_COMMA) {
        return args();
    } else if (GET_TYPE == TOK_RBRACKET) {
        if (!strcmp(p_helper->func->key.str, "write")) {
            // special case for write functions - variadic functions
            // parameters are not checked
            return ret;
        }

        //if (!str_isequal(p_helper->func->params, p_helper->temp)) {
        //    // function call does not match with definition params
        //    return ERROR_SEMANTIC_PARAMS;
        //}
        if (p_helper->func->params.length != p_helper->temp.length)
            return ERROR_SEMANTIC_PARAMS;
        for (unsigned i = 0; i < p_helper->temp.length; i++) {
            if ((p_helper->func->params.str[i] != p_helper->temp.str[i]) &&
               ((p_helper->temp.str[i] == 'i') && (p_helper->func->params.str[i] == 'n'))) {
                generate_num_conversion(i);
            } else if ((p_helper->func->params.str[i] != p_helper->temp.str[i])) {
                return ERROR_SEMANTIC_PARAMS;
            }
        }

        generate_call(p_helper);

        if (p_helper->id_first != NULL) {
            if (p_helper->id_first->data != NULL && p_helper->assign) {
                // assign function return value into variable(s)
                generate_assign_function(p_helper);
            }
        }
        return ret;
    } else {
        return ERROR_SYNTAX;
    }

}


int main() {
    int ret_main = parse();
    //fprintf(stderr, "%d", ret_main);
    return ret_main;
}
