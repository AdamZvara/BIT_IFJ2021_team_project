/**
* VUT IFJ Project 2021.
*
* @file scanner.c
*
* @brief Implementation of scanner.
*
* @author Vojtěch Eichler 
* @author Václav Korvas
* @author Tomáš Matuš
* @author Adam Zvara
*/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "scanner.h"
#include "error.h"

// TODO define states
#define STATE_START 1
#define STATE_ID_OR_KEYWORD 2
#define STATE_MINUS 3
#define STATE_COMMENT 4
#define STATE_DIV 5
#define STATE_ASSIGN 6
#define STATE_EQUAL 7
#define STATE_LESS 8
#define STATE_GREAT 9
#define STATE_NOT_EQUAL 10
#define STATE_NUMBER 11
#define STATE_NUMBER_POINT 12
#define STATE_NUMBER_DEC 13
#define STATE_NUMBER_E 14
#define STATE_NUMBER_EXP_SIGN 15
#define STATE_NUMBER_EXP_END 16
#define STATE_CONCAT 17
#define STATE_STRING 18
#define STATE_BLOCK_COMMENT_FIRST 19
#define STATE_BLOCK_COMMENT 20
#define STATE_BLOCK_COMMENT_LEAVE 21

//TODO string and espace_seq

int check_keyword(string_t* s, token_t* token) 
{
	if (!strcmp(s->str,"do")) 
		token->attribute.keyword = KW_DO;
	else if (!strcmp(s->str,"else")) 
		token->attribute.keyword = KW_ELSE;
	else if (!strcmp(s->str,"end"))
		token->attribute.keyword = KW_END;
	else if (!strcmp(s->str,"function"))
		token->attribute.keyword = KW_FUNCTION;
	else if (!strcmp(s->str,"global"))
		token->attribute.keyword = KW_GLOBAL;
	else if (!strcmp(s->str,"if"))
		token->attribute.keyword = KW_IF;
	else if (!strcmp(s->str,"integer"))
		token->attribute.keyword = KW_INTEGER;
	else if (!strcmp(s->str,"local"))
		token->attribute.keyword = KW_LOCAL;
	else if (!strcmp(s->str,"nil"))
		token->attribute.keyword = KW_NIL;
	else if (!strcmp(s->str,"number"))
		token->attribute.keyword = KW_NUMBER;
	else if (!strcmp(s->str,"require"))
		token->attribute.keyword = KW_REQUIRE;
	else if (!strcmp(s->str,"return"))
		token->attribute.keyword = KW_RETURN;
	else if (!strcmp(s->str,"string"))
		token->attribute.keyword = KW_STRING;
	else if (!strcmp(s->str,"then"))
		token->attribute.keyword = KW_THEN;
	else if (!strcmp(s->str,"while"))
		token->attribute.keyword = KW_WHILE;
	else token->type = TOK_ID;

	if (token->type != TOK_ID) {
		token->type = TOK_KEYWORD;
		return SUCCESS;
	}
	
	if (str_copy(s, token->attribute.s)) {
		return ERROR_INTERNAL;
	}
	return SUCCESS;
}

// function to convert char* to int number
int convert_to_int(token_t* token, string_t* s) 
{
	int result = 0;
	char *ptr;

	result = strtol(s->str, &ptr, 10);
	
	token->type = TOK_INT;
	token->attribute.number = result;
	return SUCCESS;
}

int convert_to_double(token_t* token, string_t* s)
{
	double result = 0;
	char *ptr;

	result = strtod(s->str, &ptr);

	token->type = TOK_DECIMAL;
	token->attribute.decimal = result;
	return SUCCESS;
}


int get_token(token_t *token) 
{
	FILE *f = stdin;
	string_t s;
	string_t* str = &s;

	if (str_init(str)) {
		return ERROR_INTERNAL;
	}

	int scanner_state = STATE_START;
	token->type = TOK_NOTHING;
	char c = '\0';
	char escape_seq[2] = {'\0'};
	
	while(1) {
		c = getc(f);
		switch(scanner_state) {
			case STATE_START:
				if (c == '\n') {
					token->type = TOK_EOL;
					return SUCCESS;

				} else if (isspace(c)) {
					scanner_state = STATE_START;

				} else if (c == '(') {
					token->type = TOK_LBRACKET;
					return SUCCESS;

				} else if (c == ')') {
					token->type = TOK_RBRACKET;
					return SUCCESS;

				} else if (c == '{') {
					token->type = TOK_RCURLBRACKET;
					return SUCCESS;

				} else if (c == '}') {
					token->type = TOK_LCURBRACKET;
					return SUCCESS;

				} else if (c == '+') {
					token->type = TOK_PLUS;
					return SUCCESS;

				} else if (c == '#') {
					token->type = TOK_LEN;
					return SUCCESS;

				} else if (c == '*') {
					token->type = TOK_MUL;
					return SUCCESS;

				} else if (c == '/') {
					scanner_state = STATE_DIV;

				} else if (c == '-') {
					scanner_state = STATE_MINUS;

				} else if (c == '.') {
					scanner_state = STATE_CONCAT;

				} else if (c == '~') {
					scanner_state = STATE_NOT_EQUAL;

				} else if (c == '>') {
					scanner_state = STATE_GREAT;

				} else if (c == '<') {
					scanner_state = STATE_LESS;
				
				} else if (c == '=') {
					scanner_state = STATE_EQUAL;

				} else if (c == ':') {
					token->type = TOK_COLON;
					return SUCCESS;
				
				} else if (c == ',') {
					token->type = TOK_COMMA;
					return SUCCESS;

				} else if (c == EOF) {
					token->type = TOK_EOF;
					return SUCCESS;

				} else if (isalpha(c) || c == '_') {
					
					if (str_add_char(str, c)) {
						return ERROR_INTERNAL;
					}
					scanner_state = STATE_ID_OR_KEYWORD;

				} else if (isdigit(c)) {
					
					if (str_add_char(str, c)) {
						return ERROR_INTERNAL;
					}
					scanner_state = STATE_NUMBER;
				
				} else if (c == '"') {
					scanner_state = STATE_STRING;

				} else {

					return ERROR_LEXICAL;
				}

				break; // end of START_STATE
			
			case STATE_DIV:
				if (c == '/') {
					token->type = TOK_INT_DIV;

				} else {
					ungetc(c, f);
					token->type = TOK_DIV;

				}
				return SUCCESS;
				break;

			case STATE_CONCAT:
				if (c == '.') {
					token->type= TOK_CONCAT;
					return SUCCESS;

				} else {
					ungetc(c, f);
					return ERROR_LEXICAL;
				
				}
				break;

			case STATE_NOT_EQUAL:
				if (c == '=') {
					token->type = TOK_NEQ;
					return SUCCESS;
				
				} else {
					ungetc(c, f);
					return ERROR_LEXICAL;
				}
				break;

			case STATE_GREAT:
				if (c == '=') {
					token->type = TOK_GR_EQ;
					
				} else {
					ungetc(c, f);
					token->type = TOK_GR;
				}
				return SUCCESS;
				break;

			case STATE_LESS:
				if (c == '=') {
					token->type = TOK_LES_EQ;

				} else {
					token->type = TOK_LES;
				}
				return SUCCESS;
				break;

			case STATE_EQUAL:
				if (c == '=') {
					token->type = TOK_EQ;
					
				} else {
					token->type = TOK_ASSIGN;

				}
				return SUCCESS;
				break;

			// state for id and keyword proccessing
			case STATE_ID_OR_KEYWORD:
				if (isalnum(c) || c == '_') {
					if (str_add_char(str, c)) {
						return ERROR_INTERNAL;
					}

				} else {
					ungetc(c, f);
					return check_keyword(str, token);

				}
				break;

			// states for number proccessing
			case STATE_NUMBER:
				if (isdigit(c)) {
					if (str_add_char(str, c)) {
						return ERROR_INTERNAL;
					}

				} else if (c == '.') {
					scanner_state = STATE_NUMBER_POINT;
					if (str_add_char(str, c)) {
						return ERROR_INTERNAL;
					}

				} else if (c == 'e' || c == 'E') {
					scanner_state = STATE_NUMBER_E;
					if (str_add_char(str, c)) {
						return ERROR_INTERNAL;
					}

				} else {
					ungetc(c, f);
					return convert_to_int(token, str);
				}
				break;

			case STATE_NUMBER_POINT:
				if (isdigit(c)) {
					if (str_add_char(str, c)) {
						return ERROR_INTERNAL;
					}
					scanner_state = STATE_NUMBER_DEC;

				} else {
					ungetc(c, f);
					return ERROR_LEXICAL;
				}

				break;

			case STATE_NUMBER_DEC:
				if (isdigit(c)) {
					if (str_add_char(str, c)) {
						return ERROR_INTERNAL;
					}

				} else if (c == 'E' || c == 'e') {
					scanner_state = STATE_NUMBER_E;
					if (str_add_char(str, c)) {
						return ERROR_INTERNAL;
					}

				} else {
					ungetc(c, f);
					return convert_to_double(token, str);
				}

				break;

			case STATE_NUMBER_E:
				if (isdigit(c)) {
					scanner_state = STATE_NUMBER_EXP_END;
					if (str_add_char(str, c)) {
						return ERROR_INTERNAL;
					}
					
				} else if (c == '+' || c == '-') {
					scanner_state = STATE_NUMBER_EXP_SIGN;
					if (str_add_char(str, c)) {
						return ERROR_INTERNAL;
					}

				} else {
					ungetc(c, f);
					return ERROR_LEXICAL;
				}
				
				break;

			case STATE_NUMBER_EXP_SIGN:
				if (isdigit(c)) {
					if (str_add_char(str, c)) {
						return ERROR_INTERNAL;
					}

				} else {
					ungetc(c, f);
					return ERROR_LEXICAL;
				}

				break;

			case STATE_NUMBER_EXP_END:
				if (isdigit(c)) {
					if (str_add_char(str, c)) {
						return ERROR_INTERNAL;
					}

				} else {
					ungetc(c, f);
					return convert_to_double(token, str);
				
				}

				break;

			// states for commentary or minus sign (-)
			case STATE_MINUS:
				if (c == '-') {
					scanner_state = STATE_COMMENT;

				} else {
					token->type = TOK_MINUS;
					ungetc(c, f);
					return SUCCESS;
				}
				break;

			case STATE_COMMENT:
				if (c == '\n') {
					scanner_state = STATE_START;

				} else if (c == '[') {
					scanner_state = STATE_BLOCK_COMMENT_FIRST;
				}
				break;

			case STATE_BLOCK_COMMENT_FIRST:
				if (c == '[') {
					scanner_state = STATE_BLOCK_COMMENT;
				
				} else if (c == '\n') {
					scanner_state = STATE_START;
					ungetc(c, f);
					return SUCCESS;
		
				} else if (c != '\n') {
					scanner_state = STATE_COMMENT;

				}
				break;

			case STATE_BLOCK_COMMENT:
				if (c == ']') {
					scanner_state = STATE_BLOCK_COMMENT_LEAVE;
				}
				break;

			case STATE_BLOCK_COMMENT_LEAVE:
				if (c == ']') {
					scanner_state = STATE_START; 
				} else {
					scanner_state = STATE_BLOCK_COMMENT;
				}
				break;

			// state for string literal
			case STATE_STRING:
				//TODO
				break;
		} // case
	} // while loop
	return SUCCESS; 
}
