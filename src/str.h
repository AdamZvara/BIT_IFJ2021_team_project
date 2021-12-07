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

/**
 * @brief Inicialization of string struct.
 *
 * @param s Pointer to the string.
 *
 * @return 0 if successful, else return 1.
 */
int str_init(string_t* s);

/**
 * @brief Free allocated string
 *
 * @param s Pointer to the string.
 *
 */
void str_free(string_t* s);

/**
 * @brief Add char to the end of string
 *
 * @param s Pointer to the string.
 * @param c The added character
 *
 * @return 0 if successful, else return 1.
 */
int str_add_char(string_t* s, char c);

/**
 * @brief Insert string into string_t
 *
 * @param str	 String structure to insert into
 * @param insert String to insert
 *
 * @return 0 if successful, else return 0
 */
int str_insert(string_t* str, char* insert);

/**
 * @brief Insert integer value to string_t
 *
 * @param str String to insert into
 * @param num Number to insert
 *
 * @return 0 if successful, else return 1
 */
int str_insert_int(string_t* str, int num);

/**
 * @brief Insert double value to string_t
 *
 * @param str String to insert into
 * @param num Number to insert
 *
 * @return 0 if successful, else return 1
 */
int str_insert_double(string_t* str, double num);

/**
 * @brief Copy string from source to destination
 *
 * @param source Source string to be copied
 * @param destination Destination string
 *
 * @return 0 if successful, else return 1.
 */
int str_copy(string_t* source, string_t* destination);

/**
 * @brief Clear string
 *
 * @param src Pointer to string to clear
 */
void str_clear(string_t *src);

/**
 * @brief Compare 2 strings
 *
 * @param src First string
 * @param dst Second string
 *
 * @return 1 if stings are equal, otherwise 0
 */
int str_isequal(const string_t src, const string_t dst);

/**
 * @brief Check if string is empty
 *
 * @param src String
 *
 * @return 1 if string is empty, otherwise 0
 */
int str_empty(const string_t src);

/**
 * @brief Get string length
 *
 * @param src String
 *
 * @return Length of string
 */
int str_len(const string_t src);

/**
 * @brief Return last char of string
 *
 * @param src String
 *
 * @return Last character
 */
char str_getlast(const string_t src);

/**
 * @brief Delete last char of string
 *
 * @param src String
 */
void str_clearlast(string_t* src);

/**
 * @brief Look for a character in string
 *
 * @param src String
 * @param c Character to look for
 *
 * @return 1 if string contains given character, otherwise 0
 */
int str_contains(const string_t src, char c);

#endif //_STR_H_
