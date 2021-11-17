/**
 * VUT IFJ Project 2021.
 *
 * @file generator.h
 * 
 * @brief Header file for generator.c
 * 
 * @author Vojtěch Eichler 
 * @author Václav Korvas
 * @author Tomáš Matuš
 * @author Adam Zvara
 */

#ifndef _GENERATOR_H 
#define _GENERATOR_H

#include <stdio.h>

// macro for generating instruction call
// GENERATE(test) calls function generate_test
#define GENERATE(instruction, var)           \
do {                                    \
    generate_ ## instruction(var);         \
} while(0)                              \

void generate_start();
void generate_decvar(char *name);
void generate_fdef();
void generate_fargs(char *name);

#endif // _GENERATOR_H