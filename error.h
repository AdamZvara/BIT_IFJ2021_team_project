/**
 * VUT IFJ Project 2021.
 *
 * @file error.h
 *
 * @brief Error return codes definition.
 *
 * @author Vojtěch Eichler
 * @author Václav Korvas
 * @author Tomáš Matuš
 * @author Adam Zvara
 */

#ifndef _EROR_H_
#define _EROR_H_

#define SUCCESS 	            0 // Successful return code
#define ERROR_LEXICAL 		    1 // Lexical analysis error
#define ERROR_SYNTAX 		    2 // Syntax analysis error
#define ERROR_SEMANTIC 		    3 // Semantic error (undefined function/variable, redefinition)
#define ERROR_SEMANTIC_ASSIGN       4 // Semantic error during assignment (type incompatibility)
#define ERROR_SEMANTIC_PARAMS       5 // Semantic error (wrong arguments, argument types, return code)
#define ERROR_SEMANTIC_TYPE         6 // Semantic error - aritmetic incompatibility etc.
#define ERROR_SEMANTIC_OTHER        7 // Other semantic errors
#define ERROR_NIL 		    8 // Error while working with nil value
#define ERROR_ZERO_DIVISION         9 // Division with zero
#define ERROR_INTERNAL 		    99 // Compiler internal error

#endif // _ERROR_H_
