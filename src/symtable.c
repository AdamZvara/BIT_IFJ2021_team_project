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
#include "error.h"

int hash_function(string_t str)
{
	long long h=0;
	const unsigned char *p;
	for(p=(const unsigned char*)str.str; *p!='\0'; p++)
		h = 65599*h + *p;
	return h % GLOBAL_SYM_SIZE;
}

global_symtab_t *global_create()
{
	global_symtab_t *table;

	// if allocation fails, return NULL, allocating space for hash table +
	// n * pointer to hash table item/record
	if (!(table = malloc(sizeof(*table) + GLOBAL_SYM_SIZE * sizeof(struct global_item*))))
		return NULL;

	// hash table initialization
	table->size = GLOBAL_SYM_SIZE;
	for (int i = 0; i < GLOBAL_SYM_SIZE; i++)
		table->func[i] = NULL;

	return table;
}

struct global_item *global_find(global_symtab_t *gs, string_t key)
{
	// finding index of key in hash table
	int index = hash_function(key);

	// cycling through all records at index in hash table
	struct global_item *item;
	item = gs->func[index];
	while (item) {
		// if current item has the wanted key
		if (str_isequal(key, item->key))
			return item;
		item = item->next;
	}

	// hash table doesn't contain key
	return NULL;
}

struct global_item *global_create_fun(string_t key)
{
    struct global_item *new_func = malloc(sizeof(*new_func));

	// initialize all values
	new_func->defined = false;
	if (str_init(&new_func->key)) return NULL;
	if (str_init(&new_func->retvals)) return NULL;
	if (str_init(&new_func->params)) return NULL;
	new_func->next = NULL;

	// copy key to function key
	if (str_copy(&key, &new_func->key)) return NULL;

	return new_func;
}


struct global_item *global_add(global_symtab_t *gs, string_t key)
{
	// try to find if function already exists and return pointer to it
	struct global_item *find = global_find(gs, key);
	if (find != NULL) {
		return find;
	}

	// create new function and insert it at the beginning of list
	int hash = hash_function(key);
	struct global_item *new_func = global_create_fun(key);
	if (new_func == NULL) {
		return NULL;
	}

	new_func->next = gs->func[hash];
	gs->func[hash] = new_func;

	return new_func;
}

void global_destroy(global_symtab_t *gs)
{
	struct global_item *tmp, *del;

	// free all link-listed functions
	for (unsigned int i = 0; i < gs->size; i++) {
		tmp = gs->func[i];
		while (tmp != NULL) {
			del = tmp;
			tmp = tmp->next;
			// free all allocated strings
			str_free(&del->key);
			str_free(&del->params);
			str_free(&del->retvals);
			free(del);
		}
	}

	free(gs);
}

local_symtab_t *local_create(string_t key)
{
	// allocate memory for table
	local_symtab_t *local = malloc(sizeof(*local) + LOCAL_SYM_SIZE * sizeof(struct local_data*));
	if (local == NULL) {
		return NULL;
	}

	// initialize values
	if (str_init(&local->key)) return NULL;
	if (str_copy(&key, &local->key)) return NULL;
	local->depth = 0;
	local->size = 0;
	local->alloc_size = LOCAL_SYM_SIZE;
	local->next = NULL;

	for (unsigned int i = 0; i < local->alloc_size; i++) {
		local->data[i] = NULL;
	}

	return local;
}

int local_new_depth(local_symtab_t **previous)
{
	if (*previous == NULL) {
		return ERROR_INTERNAL;
	}

	// create new local TS with same name as previous
	local_symtab_t *new_local = local_create((*previous)->key);
	if (new_local == NULL) {
		return ERROR_INTERNAL;
	}

	new_local->depth++;

	new_local->next = *previous;
	*previous = new_local;

	return 0;
}

struct local_data *local_add(local_symtab_t *local_tab, string_t name, bool init)
{
	// create pointer to data
	struct local_data **id = &(local_tab->data[local_tab->size]);
	*id = malloc(sizeof(struct local_data));
	if (id == NULL) {
		return NULL;
	}

	// fill pointer with information
	if (str_init(&(*id)->name)) return NULL;
	if (str_copy(&name, &(*id)->name)) return NULL;
	(*id)->init = init;
	(*id)->type = NIL_T;

	local_tab->size++;
	// TODO: realloc if table is full
	return *id;
}

void local_add_type(struct local_data *data, keyword_t kw)
{
	if (kw == KW_STRING) {
		data->type = STR_T;
	} else if (kw == KW_NUMBER) {
		data->type = NUM_T;
	} else if (kw == KW_INTEGER) {
		data->type = INT_T;
	} else if (kw == KW_NIL) {
		data->type = NIL_T;
	}
}

struct local_data *local_find(local_symtab_t *local_tab, string_t name)
{
	local_symtab_t *tmp = local_tab;
	if (tmp == NULL) {
		return NULL;
	}

	// iterate through all local symtabs with the same key
	while (true) {
		// search through identifiers in this local symtable
		for (unsigned int i = 0; i < tmp->size; i++) {
			if (str_isequal(tmp->data[i]->name, name)) {
				return tmp->data[i];
			}
		}

		// end of linked list
		if (tmp->next == NULL) {
			return NULL;
		}

		// check if next local symtab has different key
		if (!str_isequal(tmp->key, tmp->next->key)) {
			return NULL;
		}
		tmp = tmp->next;
	}
}

void local_delete_top(local_symtab_t **local_tab)
{
	if (*local_tab == NULL) {
		return;
	}

	// store top of local symtable to del pointer
	// move local top to the next local symtable
	local_symtab_t *del;
	del = *local_tab;
	*local_tab = (*local_tab)->next;

	// free key of local symtable
	str_free(&del->key);
	for (unsigned int i = 0; i < del->size; i++) {
		// free names of all identifiers
		str_free(&(del->data[i]->name));
		// free local data structure
		free(del->data[i]);
	}
	// free the local table itself
	free(del);
}

void local_destroy(local_symtab_t *local_tab)
{
	// iterate through local symtables and destroy
	while (local_tab != NULL) {
		local_delete_top(&local_tab);
	}
}