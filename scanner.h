
#ifndef _SCANNER_H_
#define _SCANNER_H_

#include "str.h"

typedef enum {
	TOK_ID,				// Identifire
	TOK_KEYWORD,		// Keyword
	TOK_EOL,			// EOL
	TOK_EOF,			// EOF
	TOK_NOTHING,	

	TOK_EQ,				// Equal (==)
	TOK_NEQ,			// Not equal (~=)
	TOK_LES,			// Less than (<)
	TOK_LES_EQ,			// Less and equal (<=)
	TOK_GR,				// Greater than (>)
	TOK_GR_EQ,			// Greater and equal (>=)
	TOK_ASSIGN, 		// ASSIGN (=)

	TOK_LEN,			// Length (#)
	TOK_PLUS,			// +
	TOK_MINUS,			// -
	TOK_MUL,			// *
	TOK_DIV,			// Whole number  after div (/) 
	TOK_MOD,			// Remainder after div (//)
	TOK_CONCAT,			// Concatenation (..)
	
	TOK_RBRACKET, 		// )
	TOK_LBRACKET, 		// (
	TOK_RCURLBRACKET, 	// }
	TOK_LCURBRACKET, 	// {
	
	TOK_COMMA, 			// ,
	TOK_COLON, 			// :
	TOK_INT,
	TOK_DECIMAL,
	TOK_STRING,

} token_type_t;

typedef enum {
	KW_DO,
	KW_ELSE,
	KW_END,
	KW_FUNCTION,
	KW_GLOBAL,
	KW_IF,
	KW_INTEGER,
	KW_LOCAL,
	KW_NIL,
	KW_NUMBER,
	KW_REQUIRE,
	KW_RETURN,
	KW_STRING,
	KW_THEN,
	KW_WHILE,

} keyword_t;

typedef union {
	string_t* s;
	double decimal;
	int number;
	keyword_t keyword;
} token_attribute_t;

typedef struct {
	token_attribute_t attribute;
	token_type_t type;
} token_t;



int get_token(token_t* token);

#endif //_SCANNER_H_
