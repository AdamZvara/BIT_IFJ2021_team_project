/**
 * VUT IFJ Project 2021.
 *
 * @file func_helper.h
 *
 * @brief Header file for func_helper.c
 *
 * @author Vojtěch Eichler
 * @author Václav Korvas
 * @author Tomáš Matuš
 * @author Adam Zvara
 */

#ifndef _FUNC_HELPER_
#define _FUNC_HELPER_

#include "symtable.h"   // ID TYPE, LOCAL SYMTAB
#include "str.h"        // DYNAMIC STRING
#include "scanner.h"    // TOKEN, KEYWORD TYPES

extern local_symtab_t *local_tab;

/**
 * @brief Helper structure to do semantic checks when declaring/defining new function
 */
typedef struct func_def {
    struct global_item *item;   // Pointer to function in global symtab
    bool func_found;            // Whether function was already found in global symtab
    string_t temp;              // Temporary string to fill retvals/parameters of function
    int par_counter;            // Counter of parameters
} func_def_t;

/**
 * @brief Initialize helper structure
 * @param f Pointer to helper structure
 */
int func_init(func_def_t *f);

/**
 * @brief Clear string in helper structure
 * @param f Pointer to helper structure
 */
void func_clear(func_def_t *f);

/**
 * @brief Free helper structure
 * @param f Pointer to helper structure
 */
void func_dispose(func_def_t *f);

/**
 * @brief Set parameters of function in helper structure
 * @param f Pointer to helper structure
 * @param kw Which parameter should be added (string, number, integer)
 * @todo return values from str_add_char
 */
int func_set_params(func_def_t *f, keyword_t kw);

/**
 * @brief Set retvals of function in helper structure
 * @param f Pointer to helper structure
 * @param kw Which retval should be added (string, number, integer)
 * @todo return values from str_add_char
 */
int func_set_retvals(func_def_t *f, keyword_t kw);

/**
 * @brief Fill helper temporary string when function is called
 * @param f Pointer to helper structure
 * @param type Parameter type
 * @todo return values from str_add_char
 */
int func_call_params_const(func_def_t *f, token_type_t type);

/**
 * @brief Get type of identifier into helper structure params
 * @param f Pointer to helper structure
 * @param name Name of identifier
 * @return 0 if ID was found and added, otherwise 1
 * @todo return values from str_add_char
 */
int func_call_params_id(func_def_t *f, string_t name);

#endif