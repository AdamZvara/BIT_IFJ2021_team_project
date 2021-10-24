/**
 * VUT IFJ Project 2021.
 *
 * @file str.h
 *
 * @brief Definition of struct for dynamic string.
 *
 * @author Vojtěch Eichler 
 * @author Václav Korvas
 * @author Tomáš Matuš
 * @author Adam Zvara
 */

#ifndef _STR_H_
#define _STR_H_


typedef struct {
	char *str; 					// string ended with '\0'
	unsigned int length; 		// real length of the string
	unsigned int alloc_size; 	// allocated space for the string
} string_t;

/*
 * @brief Inicialization of string struct.
 *
 * @param s Pointer to the string.
 *
 * @return 0 if successfull, else return 1.
 */
int str_init(string_t* s);

/*
 * @brief Free allocated string
 *
 * @param s Pointer to the string.
 *
 */
void str_free(string_t* s);

/*
 * @brief Add char to the end of string
 *
 * @param s Pointer to the string.
 * @param c The added character
 *
 * @return 0 if successfull, else return 1.
 */
int str_add_char(string_t* s, char c);

/*
 * @brief Copy string from source to destination
 *
 * @param source Source string to be copied
 * @param destination Destination string
 *
 * @return 0 if successfull, else return 1.
 */
int str_copy(string_t* source, string_t* destination);
#endif //_STR_H_
