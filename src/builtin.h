/**
 * VUT IFJ Project 2021.
 *
 * @file builtin.h
 *
 * @brief Header file for builtin functions
 *
 * @author Vojtěch Eichler
 * @author Václav Korvas
 * @author Tomáš Matuš
 * @author Adam Zvara
 */

#ifndef _BUILTIN_H
#define _BUILTIN_H

#include "symtable.h"

/**
 * @brief Structure representing which builtin functions have been used
 */
typedef struct builtin_used {
    bool reads;
    bool readn;
    bool readi;
    bool tointeger;
    bool substr;
    bool ord;
    bool chr;
} builtin_used_t;

/**
 * @brief Create builtin used structure and initialize its values
 * @return Created structure
 */
builtin_used_t *builtin_used_create();

/**
 * @brief Check if any builtin function was used and set its value to true
 * @param bu Pointer to buildin_used structure
 * @param name Name of used function
 */
void builtin_used_update(builtin_used_t *bu, string_t name);

/**
 * @brief Generate used builtin functions
 * @param bu Pointer to buildin_used structure
 */
void generate_builtin(builtin_used_t *bu);

/**
 * @brief Add builtin functions to global symtab
 * @param gs Pointer to global symtab
 */
void add_builtin(global_symtab_t *gs);

/**
 * @brief Destroy builtin structure
 * @param bu Pointer to builtin structure
 */
void builtin_destroy(builtin_used_t *bu);

void generate_reads();
void generate_readi();
void generate_readn();
void generate_chr();
void generate_ord();
void generate_substr();
void generate_tointeger();


#endif // _BUILTIN_H