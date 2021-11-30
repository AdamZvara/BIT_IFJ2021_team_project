/**
 * VUT IFJ Project 2021.
 *
 * @file p_helper_helper.h
 *
 * @brief Header file for p_helper_helper.c
 *
 * @author Vojtěch Eichler
 * @author Václav Korvas
 * @author Tomáš Matuš
 * @author Adam Zvara
 */

#ifndef _p_helper_HELPER_
#define _p_helper_HELPER_

#include "symtable.h"   // ID TYPE, LOCAL SYMTAB
#include "str.h"        // DYNAMIC STRING
#include "scanner.h"    // TOKEN, KEYWORD TYPES

extern local_symtab_t *local_tab;

typedef enum {NONE, IF, WHILE} if_while;

struct identifiers {
    struct local_data *data;
    struct identifiers *next;
};

/**
 * @brief Helper structure to do semantic checks when declaring/defining new function
 */
typedef struct parser_helper {
    struct global_item *func;           // Pointer to function in global symtab
    bool func_found;                    // Whether function was already found in global symtab
    bool assign;
    struct identifiers *id_first;       // Pointer to first identificator in linked list
    struct identifiers *id_last;        // Pointer to last identificator in linked list
    int par_counter;                    // Counter of parameters
    string_t temp;                      // Temporary string to fill retvals/parameters of function
    if_while status;                    // Bool to determine whether we are in if or while statement
} parser_helper_t;

/**
 * @brief Initialize helper structure
 * @param f Pointer to helper structure
 */
parser_helper_t *p_helper_create();

/**
 * @brief Clear whole helper structure
 * @param f Pointer to helper structure
 */
void p_helper_clear(parser_helper_t *f);

/**
 * @brief Clear string in parser helper
 * @param f Pointer to helper structure
 */
void p_helper_clear_string(parser_helper_t *f);

/**
 * @brief Free helper structure
 * @param f Pointer to helper structure
 */
void p_helper_dispose(parser_helper_t *f);

/**
 * @brief Add identifer to linked list at last position
 * @param f Pointer to helper structure
 * @param id Identifier in local symtable to insert
 */
int p_helper_add_identifier(parser_helper_t *f, struct local_data *id);

/**
 * @brief Delete first identifier in linked list of identifiers
 * @param f Pointer to helper structure
 */
void p_helper_delete_identifier(parser_helper_t *f);

/**
 * @brief Set parameters of function in helper structure
 * @details If function was not declared before, set them directly into
 *  f->func parameters, otherwise set them into f->temp
 * @param f Pointer to helper structure
 * @param kw Which parameter should be added (string, number, integer)
 * @todo return values from str_add_char
 */
int p_helper_set_params(parser_helper_t *f, keyword_t kw);

/**
 * @brief Set retvals of function in helper structure
 * @param f Pointer to helper structure
 * @param kw Which retval should be added (string, number, integer)
 * @todo return values from str_add_char
 */
int p_helper_set_retvals(parser_helper_t *f, keyword_t kw);

/**
 * @brief Fill helper temporary string when function is called
 * @param f Pointer to helper structure
 * @param type Parameter type
 * @todo return values from str_add_char
 */
int p_helper_call_params_const(parser_helper_t *f, token_type_t type);

/**
 * @brief Get type of identifier into helper structure params
 * @param f Pointer to helper structure
 * @param name Name of identifier
 * @return 0 if ID was found and added, otherwise 1
 * @todo return values from str_add_char
 */
int p_helper_call_params_id(parser_helper_t *f, string_t name);

#endif