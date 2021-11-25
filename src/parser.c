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


token_t *curr_token = NULL;
token_t *backup_token = NULL;
token_t *ret_token = NULL;
global_symtab_t *global_tab = NULL;
local_symtab_t *local_tab = NULL;
ibuffer_t *buffer = NULL;
builtin_used_t *builtin_used = NULL;
int ret = SUCCESS;

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

    ret = require();

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
    func_def_t f_helper;
    func_init(&f_helper);

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

            // add function to global symtable
            f_helper.item = global_find(global_tab, GET_ID);
            if (f_helper.item != NULL) {
                // multiple declarations of function
                return ERROR_SEMANTIC;
            }
            f_helper.item = global_add(global_tab, GET_ID);

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
            ret = params(&f_helper);
            if (ret)
                return ret;

            // step into <ret_params> rule
            ret = ret_params(&f_helper);
            if (ret)
                return ret;

            func_dispose(&f_helper);
            return prog();

        } else if (GET_KW == KW_FUNCTION) { // check if keyword is _function_
            // get new token that should be ID
            NEXT_TOKEN();
            if (GET_TYPE != TOK_ID)
                return ERROR_SYNTAX;

            // check if function is already in global symtable
            f_helper.item = global_find(global_tab, GET_ID);
            if (f_helper.item == NULL) {
                // create new record in global symtable
                f_helper.item = global_add(global_tab, GET_ID);
            } else {
                f_helper.func_found = true;
            }

            // create local symtable for function
            local_tab = local_create(GET_ID);

            // get new token that should be opening bracket
            NEXT_TOKEN();
            if (GET_TYPE != TOK_LBRACKET)
                return ERROR_SYNTAX;

            ret = params_2(&f_helper);
            if (ret)
                return ret;

            // clear string containing temporary params/retvals
            func_clear(&f_helper);

            // step into <ret_params> rule
            ret = ret_params(&f_helper);
            if (ret)
                return ret;

            // generate function label + retvals and parameters
            generate_function();

            ret = body();
            if (ret)
                 return ret;

            func_dispose(&f_helper);
            return prog();

        } else { // unexpected keyword, return error
            return ERROR_SYNTAX;
        }
    } else if (GET_TYPE == TOK_ID) { // new token is ID = function call
        // try to find function in global symtable
        f_helper.item = global_find(global_tab, GET_ID);
        // if function was not found or was not defined
        if (f_helper.item == NULL) {
            return ERROR_SEMANTIC;
        }

        builtin_used_update(builtin_used, f_helper.item->key);

        NEXT_TOKEN();
        if (GET_TYPE != TOK_LBRACKET)
            return ERROR_SYNTAX;

        // no entry label was created, create one
        if (!entry) {
            generate_entry();
            entry = true;
        }

        // dont create new frame if function is write
        if (strcmp(f_helper.item->key.str, "write")) {
            generate_call_prep(&f_helper);
        }

        // <args>
        ret = args(&f_helper);
        if (ret)
            return ret;

        return prog();

    }  else if (GET_TYPE == TOK_EOF) {
        // end of input, start returning all the way up the recursion
        func_dispose(&f_helper);

        // generate end label to skip builtin functions
        generate_end();

        // generate used builtin functions
        builtin_used_generate(builtin_used);

        // generate end label
        generate_exit();

        return ret;
    } else { // unexpected token, return error
        return ERROR_SYNTAX;
    }
}

int params(func_def_t *f_helper) {
    NEXT_TOKEN();
    if (GET_TYPE == TOK_RBRACKET) {
        return ret;
    }

    if ((GET_TYPE == TOK_KEYWORD && GET_KW == KW_STRING) ||
        (GET_TYPE == TOK_KEYWORD && GET_KW == KW_NUMBER) ||
        (GET_TYPE == TOK_KEYWORD && GET_KW == KW_INTEGER)) {
        func_set_params(f_helper, GET_KW);
        return params_n(f_helper);
    } else {
        return ERROR_SYNTAX;
    }
}

int params_n(func_def_t *f_helper) {
    NEXT_TOKEN();
    if (GET_TYPE == TOK_RBRACKET) {
        return ret;
    } else if (GET_TYPE == TOK_COMMA) {
        NEXT_TOKEN();
        if ((GET_TYPE == TOK_KEYWORD && GET_KW == KW_STRING) ||
            (GET_TYPE == TOK_KEYWORD && GET_KW == KW_NUMBER) ||
            (GET_TYPE == TOK_KEYWORD && GET_KW == KW_INTEGER)) {
            func_set_params(f_helper, GET_KW);
            return params_n(f_helper);
        }  else {
            return ERROR_SYNTAX;
        }
    } else {
        return ERROR_SYNTAX;
    }
}

int params_2(func_def_t *f_helper) {
    // first check if params are empty or not
    NEXT_TOKEN();
    if (GET_TYPE == TOK_RBRACKET) {
        if (f_helper->func_found) {
            if (!str_empty(f_helper->item->params)) {
                // params are empty but helper temp is not
                return ERROR_SEMANTIC;
            }
        }
        return ret;

    } else if (GET_TYPE == TOK_ID) { // params start correctly with ID
        // add identifier to local symtable
        struct local_data *id = local_add(local_tab, GET_ID, true);

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
            func_set_params(f_helper, GET_KW);
            local_add_type(id, GET_KW);
            return params_2_n(f_helper);
        } else {
            return ERROR_SYNTAX;
        }
    } else {
        return ERROR_SYNTAX;
    }
}

int params_2_n(func_def_t *f_helper) {
    // check for end of params
    NEXT_TOKEN();
    if (GET_TYPE == TOK_RBRACKET) {
        if (f_helper->func_found) {
            // function is in global table, check if parameters match
            if (!str_isequal(f_helper->item->params, f_helper->temp)) {
                return ERROR_SEMANTIC;
            }
        }
        return ret;

    } else if (GET_TYPE == TOK_COMMA) { // check for comma
        // check the rest of the syntax and descend into parama_2_n if everything is correct
        NEXT_TOKEN();
        if (GET_TYPE != TOK_ID)
            return ERROR_SYNTAX;

        // add identifier to local symtable
        struct local_data *id = local_add(local_tab, GET_ID, true);

        NEXT_TOKEN();
        if (GET_TYPE != TOK_COLON)
            return ERROR_SYNTAX;

        // next lines check <types_keyword> rule
        NEXT_TOKEN();
        if ((GET_TYPE == TOK_KEYWORD && GET_KW == KW_STRING) ||
            (GET_TYPE == TOK_KEYWORD && GET_KW == KW_NUMBER) ||
            (GET_TYPE == TOK_KEYWORD && GET_KW == KW_INTEGER)) {
            // add parameters to global symtable
            func_set_params(f_helper, GET_KW);
            local_add_type(id, GET_KW);
            return params_2_n(f_helper);
        } else {
            return ERROR_SYNTAX;
        }
    } else {
        return ERROR_SYNTAX;
    }
}

int ret_params(func_def_t *f_helper) {
    NEXT_TOKEN();
    if (GET_TYPE != TOK_COLON) {
        // function is in global table, check if retvals match
        // current token is COLON so retvals should be empty
        if (f_helper->func_found) {
            if (str_empty(f_helper->item->retvals)) {
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
        func_set_retvals(f_helper, GET_KW);
        return ret_params_n(f_helper);
    }  else {
        return ERROR_SYNTAX;
    }
}

int ret_params_n(func_def_t *f_helper) {
    NEXT_TOKEN();
    if (GET_TYPE != TOK_COMMA) {
        // function is in global table, check if retvals match
        // retvals are not empty, compare them with f_helper temporary string
        if (f_helper->func_found) {
            if (!str_isequal(f_helper->item->params, f_helper->temp)) {
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
        func_set_retvals(f_helper, GET_KW);
        return ret_params_n(f_helper);
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

    // print out instruction buffer
    ibuffer_print(buffer);
    ibuffer_clear(buffer);

    if (GET_TYPE == TOK_KEYWORD) {
        switch (GET_KW) {
            case KW_LOCAL:  // Declaration of local variable
                NEXT_TOKEN();
                if (GET_TYPE != TOK_ID)
                    return ERROR_SYNTAX;

                // add identifer to local symtable
                struct local_data *id = local_add(local_tab, GET_ID, false);
                generate_identifier(GET_ID);

                NEXT_TOKEN();
                if (GET_TYPE != TOK_COLON)
                    return ERROR_SYNTAX;
                NEXT_TOKEN();
                if ((GET_TYPE == TOK_KEYWORD && GET_KW == KW_STRING) ||
                   (GET_TYPE == TOK_KEYWORD && GET_KW == KW_NUMBER) ||
                   (GET_TYPE == TOK_KEYWORD && GET_KW == KW_INTEGER)) {
                    local_add_type(id, GET_KW);
                    ret = init(id);
                    if (ret)
                        return ret;
                }
                return body();
                break;
            case KW_IF: // IF <expr> THEN <body> ELSE <body> END <body>
                // add new depth so local variables can be recognized
                local_new_depth(&local_tab);
                // update if counter
                local_add_if(local_tab);

                // call expression()
                ret = expression(&backup_token);
                FREE_TOK_STRING();
                free(curr_token);
                curr_token = backup_token;
                if (ret == EC_FUNC) {
                    return ERROR_SYNTAX;
                } else if (ret != SUCCESS)
                    return ret;

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
                // add new depth so local variables can be recognized
                local_new_depth(&local_tab);

                // call expr()
                ret = expression(&backup_token);
                FREE_TOK_STRING();
                free(curr_token);
                curr_token = backup_token;
                if (ret == EC_FUNC) {
                    return ERROR_SYNTAX;
                } else if (ret != SUCCESS)
                    return ret;

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
                    generate_if_end();
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
                return body();
                break;
            default:
                return ERROR_SYNTAX;
                break;
        }
    } else if (GET_TYPE == TOK_ID) { // ID <body_n> <body>
        func_def_t f_helper;
        func_init(&f_helper);

        // in case of function call
        f_helper.item = global_find(global_tab, GET_ID);

        // in case of ID assign
        f_helper.id = local_find(local_tab, GET_ID);

        ret = body_n(&f_helper);

        func_dispose(&f_helper);

        if (ret)
            return ret;
        return body();
    } else {
        return ERROR_SYNTAX;
    }
}

int body_n(func_def_t *f_helper) {
    if (backup_token) {
        curr_token = backup_token;
        backup_token = NULL;
    } else {
        NEXT_TOKEN();
    }

    if (GET_TYPE == TOK_LBRACKET) {
        // function is not found
        if (f_helper->item == NULL) {
            return ERROR_SEMANTIC;
        }

        builtin_used_update(builtin_used, f_helper->item->key);

        // dont create new frame if function is write
        if (strcmp(f_helper->item->key.str, "write")) {
            generate_call_prep(f_helper);
        }

        return args(f_helper);
    } else if (GET_TYPE == TOK_ASSIGN) {
        return assign_single(f_helper);
    } else if (GET_TYPE == TOK_COMMA) {
        NEXT_TOKEN();
        if (GET_TYPE != TOK_ID)
            return ERROR_SYNTAX;

        ret = assign_multi();
        if (ret)
            return ret;


        return r_side();
    } else {
        return ERROR_SYNTAX;
    }
}

int assign_single(func_def_t *f_helper) {
    // call expression()
    ret = expression(&backup_token);
    if (ret == SUCCESS) {
        generate_assign(f_helper->id->name);
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
        f_helper->item = global_find(global_tab, GET_ID);

        ret = body_n(f_helper);

        return ret;
    }
    else
        return ret;
}

int assign_multi() {
    NEXT_TOKEN();
    if (GET_TYPE == TOK_COMMA) {
        NEXT_TOKEN();
        if (GET_TYPE != TOK_ID)
            return ERROR_SYNTAX;
        return assign_multi();
    } else if (GET_TYPE == TOK_ASSIGN) {
        return ret;
    } else {
        return ERROR_SYNTAX;
    }
}

int r_side() {
    // call expression()
    ret = expression(&backup_token);
    if (ret == SUCCESS) {
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
        func_def_t f_helper;
        func_init(&f_helper);
        f_helper.item = global_find(global_tab, GET_ID);

        builtin_used_update(builtin_used, f_helper.item->key);
        
        backup_token = curr_token;
        ret = body_n(&f_helper);

        func_dispose(&f_helper);
        if (ret)
            return ret;

        return r_side_n();
    }
    else
        return ret;
}

int r_side_n() {
    //TODO: not sure about this:
    // but I think this should solve that expression reads one more token after
    // the expression itself
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

//TODO: remove this (probably useless?) function
int func() {
    // <args>
    //ret = args(); TODO:
    if (ret)
        return ret;

    return ret;
}

int init(struct local_data *id) {
    NEXT_TOKEN();
    if (GET_TYPE == TOK_ASSIGN) {
        id->init = true;
        return init_n(id);
    } else {
        backup_token = curr_token;
        return ret;
    }
}

int init_n(struct local_data *id) {
    // call expression()
    ret = expression(&backup_token);
    if (ret == SUCCESS) {
        generate_assign(id->name);
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

        // perform functino call
        func_def_t f_helper;
        func_init(&f_helper);
        f_helper.item = global_find(global_tab, GET_ID);

        builtin_used_update(builtin_used, f_helper.item->key);
        
        backup_token = curr_token;
        ret = body_n(&f_helper);

        func_dispose(&f_helper);

        return ret;
    } else
        return ret;
}

int args(func_def_t *f_helper) {
    NEXT_TOKEN();
    if (GET_TYPE == TOK_RBRACKET) {
        if (!strcmp(f_helper->item->key.str, "write")) {
            // special case for write functions - variadic functions
            // parameters are not checked
            return ret;
        }

        if (!str_isequal(f_helper->item->params, f_helper->temp)) {
                // function call does not match with definition params
                return ERROR_SEMANTIC_PARAMS;
            }
        generate_call(f_helper);
        return ret;
    }

    if (GET_TYPE == TOK_STRING || GET_TYPE == TOK_DECIMAL || GET_TYPE == TOK_INT) {
        if (!strcmp(f_helper->item->key.str, "write")) {
            // instructions are generated on spot
            generate_write(curr_token);
            return args_n(f_helper);
        }
        func_call_params_const(f_helper, GET_TYPE);
        generate_call_params(curr_token, f_helper);
        return args_n(f_helper);
    } else if (GET_TYPE == TOK_ID) {
        if (!strcmp(f_helper->item->key.str, "write")) {
            // instructions are generated on spot
            generate_write(curr_token);
            return args_n(f_helper);
        }
        func_call_params_id(f_helper, GET_ID);
        generate_call_params(curr_token, f_helper);
        return args_n(f_helper);
    } else {
        return ERROR_SYNTAX;
    }
}

int args_n(func_def_t *f_helper) {
    NEXT_TOKEN();
    if (GET_TYPE == TOK_COMMA) {
        return args(f_helper);
    } else if (GET_TYPE == TOK_RBRACKET) {
        if (!strcmp(f_helper->item->key.str, "write")) {
            // special case for write functions - variadic functions
            // parameters are not checked
            return ret;
        }

        if (!str_isequal(f_helper->item->params, f_helper->temp)) {
            // function call does not match with definition params
            return ERROR_SEMANTIC_PARAMS;
        }

        generate_call(f_helper);
        return ret;
    } else {
        return ERROR_SYNTAX;
    }

}


int main() {
    int ret_main = parse();
    fprintf(stderr, "%d", ret_main);
    return ret_main;
}
