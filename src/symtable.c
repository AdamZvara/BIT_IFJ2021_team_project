/**
 * VUT IFJ Project 2021.
 *
 * @file symtable.c
 *
 * @brief Implementation of symtable and stack used for syntactic analysis
 *
 * @author Vojtěch Eichler 
 * @author Václav Korvas
 * @author Tomáš Matuš
 * @author Adam Zvara
 */

/* Implementation of hash table was taken from Language C course,
 * original implementation was made by Vojtech Eichler (xeichl01) */

#include "symtable.h"

int hash_function(sym_key str)
{
	int h=0;
	const unsigned char *p;
	for(p=(const unsigned char*)str; *p!='\0'; p++)
		h = 65599*h + *p;
	return h % SYM_TABLE_SIZE;
}

symtab_t *symtab_init()
{
	symtab_t *table;
	// if allocation fails, return NULL, allocating space for hash table + 
	// n * pointer to hash table item/record
	if (!(table = malloc(sizeof(symtab_t) + SYM_TABLE_SIZE * sizeof(symtab_item_t*))))
		return NULL;

	// hash table initialization
	table->size = 0;
	for (int i = 0; i < SYM_TABLE_SIZE; i++)
		table->items[i] = NULL;

	return table;
}

symtab_data_t *symtab_find(symtab_t *t, sym_key key)
{
	symtab_item_t *item;
	// finding index of key in hash table
	int index = hash_function(key);

	// cycling through all records at index in hash table
	item = t->items[index];
	while (item) {
		// if current item has the wanted key
		if (!(strcmp(key, item->key)))
			return item->data;
		item = item->next;
	}

	// hash table doesn't contain key
	return NULL;
}

symtab_item_t *symtab_init_item(sym_key key)
{
    return NULL;
}

symtab_data_t *symtab_lookup_add(symtab_t *t, sym_key key)
{
    // TODO
    return NULL;
}

bool symtab_erase(symtab_t *t, sym_key key)
{
	symtab_item_t *item;
	symtab_item_t *tmp;
	// counting index by calling hash function
	int index = hash_function(key);
	
	// if first record is the one I am looking for, erase it
	// also checks wherever there is a record at index
	item = t->items[index];
	if (item) {
		if (!(strcmp(key, item->key))) {
			tmp = item->next;
			// TODO call item_destroy function
			free(item->key);
			free(item->data);
			free(item);
			t->items[index] = tmp;
			t->size--;
			return true;
		}
	} else {
		return false;
	}
	
	// cycling through the rest of records, erase if found
	while (item->next) {
		if (!(strcmp(key, item->key))) {
			tmp = item->next;
			item->next = item->next->next;
			free(tmp->key);
            free(tmp->data);
			free(tmp);
			t->size--;
			return true;
		}
		item = item->next;
	}
	// if record wasn't found
	return false;
}

void symtab_clear(symtab_t *t)
{
	symtab_item_t *item;
	symtab_item_t *tmp;
	for (size_t i = 0; i < SYM_TABLE_SIZE; i++) {
		item = t->items[i];
		while (item != NULL) {
			tmp = item->next;
			free(item->key);
			free(item->data);
			free(item);
			item = tmp;
		}
		t->items[i] = NULL;
	}
	t->size = 0;
}
