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
#define SYM_TABLE_SIZE 10781

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct symtab_data {
	// TODO
} symtab_data_t;

typedef char *sym_key;
typedef struct symtab_item symtab_item_t;

struct symtab_item {
	sym_key key;
	symtab_data_t *data;
	symtab_item_t *next;
};

typedef struct symtab {
	// number of records inside hash table
	int size;
	// flexible array member
	symtab_item_t *items[];
} symtab_t;



int hash_function(sym_key key);
symtab_t *symtab_init();
symtab_data_t * symtab_find(symtab_t * t, sym_key key);
symtab_item_t *symtab_init_item(sym_key key);
symtab_data_t * symtab_lookup_add(symtab_t * t, sym_key key);
bool symtab_erase(symtab_t * t, sym_key key);    // ruší zadaný záznam
void symtab_clear(symtab_t * t);    // ruší všechny záznamy
void symtab_free(symtab_t * t);     // destruktor tabulky

#endif
