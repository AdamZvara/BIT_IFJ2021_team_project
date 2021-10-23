/**
 * VUT IFJ Project 2021.
 *
 * @file str.c
 *
 * @brief Implementation of dynamic string.
 *
 * @author Vojtěch Eichler 
 * @author Václav Korvas
 * @author Tomáš Matuš
 * @author Adam Zvara
 */

#include <stdlib.h>
#include "str.h"
#include "error.h"
#include <string.h>

#define STR_LENGTH_INC 16 	// Length of the string when inicialized

int str_init(string_t* s)
{
	s->str = malloc(STR_LENGTH_INC);
	if (!s->str) {
		return ERROR_INTERNAL;
	}
	
	s->alloc_size = STR_LENGTH_INC;
	s->length = 0;
	s->str[s->length] = '\0';
	
	return SUCCESS; 
}

void str_free(string_t* s)
{
	free(s->str);

}

int str_add_char(string_t* s, char c)
{
	if (s->length + 1 >= s->alloc_size) {
		s->str = realloc(s->str, s->length + STR_LENGTH_INC);
		if (!s->str) {
			return ERROR_INTERNAL;
		}
		s->alloc_size += STR_LENGTH_INC;
	}
	s->str[s->length++] = c;
	s->str[s->length] = '\0';

	return SUCCESS;
}

int str_copy(string_t* source, string_t* destination)
{
	if (source->length >= destination->length) {
		destination->str = realloc(destination->str, source->length + 1);
		if (!destination->str) {
			return ERROR_INTERNAL;
		}
		destination->alloc_size = source->length + 1;
	}
	strcpy(destination->str, source->str);
	destination->length = source->length + 1;
	return SUCCESS;
}
