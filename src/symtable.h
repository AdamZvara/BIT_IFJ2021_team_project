/**
 * VUT IFJ Project 2021.
 *
 * @file symtable.h
 *
 * @brief Definition of symtable and stack used for syntactic analysis
 *
 * @author Vojtěch Eichler
 * @author Václav Korvas
 * @author Tomáš Matuš
 * @author Adam Zvara
 */

/* Implementation of hash table was taken from Language C course,
 * original implementation was made by Vojtech Eichler (xeichl01) */

#ifndef _SYMTABLE_H_
#define _SYMTABLE_H_

#define GLOBAL_SYM_SIZE 20
#define LOCAL_SYM_SIZE	20

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "scanner.h"	// keyword_t for variable
#include "str.h"

// Identificator types
typedef enum type_t {
	STR_T,
	INT_T,
	NUM_T,
	NIL_T
} type_t;

/**
 * @brief Information about identificators
 */
struct local_data {
	string_t name;
	type_t type;
	bool init;
};

/**
 * @brief Information about local symtable for single function
 */
typedef struct local_symtab {
	string_t key;
	unsigned int depth;				// Level of depth (if, while ...)
	unsigned int size;				// Amount of stored variables
	unsigned int alloc_size;		// Size of allocated space for variables
	unsigned int if_cnt;			// Counter of if statements for unique label generation
	unsigned int while_cnt;			// Counter of while statements for unique label generation
	struct local_symtab *next;		// Pointer to next local TS (creating linked list)
	struct local_data *data[];		// Variables inside function
} local_symtab_t;

/**
 * @brief Information about function
 */
struct global_item {
	bool defined;
	string_t key;				// Name of function
	string_t retvals;			// Function return values in string format
	string_t params;			// Parameters of function in string format
	struct global_item *next;
	//TODO: maybe add names of parameters with their types?
};

/**
 * @brief Global symtable containing list of declared functions
 */
typedef struct global_symtab {
	unsigned int size;
	struct global_item *func[];
} global_symtab_t;

/**
 * @brief Create index to hashtable from string
 * @param key Key in string format
 * @return Index to hashtable based on key
 */
size_t hash_function(string_t key);

/**
 * @brief Create global symtable with initialized values
 * @return Initialized global symtable
 */
global_symtab_t *global_create();

/**
 * @brief Find function in global symtable
 * @param gs Pointer to global symtable
 * @param key Function name
 * @return Pointer to found function or NULL
 */
struct global_item *global_find(global_symtab_t *gs, string_t key);

/**
 * @brief Insert new function to gs, if no function with same key is found
 * @param gs Pointer to global symtable
 * @param key Function name
 * @return Pointer to found/created function
 */
struct global_item *global_add(global_symtab_t *gs, string_t key);

/**
 * @brief Destroy global symtable and free all its resources
 * @param gs Pointer to global symtable
 */
void global_destroy(global_symtab_t *gs);

// dont need these functions as they are just str_add_char()
// global_add_param = str_add_char(global_item, first char of param);
// global_add_retval = str_add_char(global_item, first char of retvals);

/**
 * @brief Create local symtable
 * @param key Name of function
 * @return Pointer to newly created local symtable
 */
local_symtab_t *local_create(string_t key);

/**
 * @brief Create new depth of last function in local symtable
 * @param previous Pointer to local symtable
 * @return 0 if successful, otherwise ERROR_INTERNAL TODO: maybe change
 */
int local_new_depth(local_symtab_t **previous);

/**
 * @brief Add new variable into local symtable (used before local_add_type)
 * @param name Name of new variable
 * @return Pointer to new variable
 */
struct local_data *local_add(local_symtab_t *local_tab, string_t name, bool init);

/**
 * @brief Add type of identifier (after local_add)
 * @param data Pointer to given id structure from local_add
 * @param kw Type of identifier
 */
void local_add_type(struct local_data *data, keyword_t kw);

/**
 * @brief Find identifier in local symtable linked list (only functions with the same name)
 * @param local_tab Pointer to local symtable
 * @param name Name of identifier
 * @return Pointer to item, if item was not found NULL
 */
struct local_data *local_find(local_symtab_t *local_tab, string_t name);

/**
 * @brief Find identifier in local symtable linked list and return local symtable frame
 * @param local_tab Pointer to local symtable
 * @param name Name of identifier
 * @return Pointer to local symtable frame, in which id was found, otherwise return NULL
 */
local_symtab_t *local_symtab_find(local_symtab_t *local_tab, string_t name);

/**
 * @brief Increase if_cnt in local symtable
 * @param local_tab Pointer to local symtable
 */
void local_add_if(local_symtab_t *local_tab);

/**
 * @brief Increase while_cnt in local symtable
 * @param local_tab Pointer to local symtable
 */
void local_add_while(local_symtab_t *local_tab);

/**
 * @brief Free top of the local symtable and all its resources
 * @param local_tab Pointer to active local symtable
 */
void local_delete_top(local_symtab_t **local_tab);

/**
 * @brief Free local symtable and all its resources
 * @param local_tab Pointer to active local symtable
 */
void local_destroy(local_symtab_t *local_tab);

#endif
