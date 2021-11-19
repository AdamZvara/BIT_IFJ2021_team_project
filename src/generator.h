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

#define IBUFFER_SIZE 200  // size of ibuffer
#define INSTR_SIZE   100  // size of single instruction

#include <stdio.h>
#include "ibuffer.h"
#include "symtable.h"

extern local_symtab_t *local_tab;   // local symtable from parser
extern global_symtab_t *global_tab; // global symtable from parser
extern ibuffer_t *buffer;           // instruction buffer from parser

/**
 * @brief Generate name of identifier
 * @param local_tab Pointer to local symtable
 * @param name      Name of variable
 * @return Unique identifier name in format func_name$depth$id_name
 */
void generate_name(local_symtab_t *local_tab, string_t name, string_t *output);

void generate_start();

void generate_label(string_t name);
void generate_parameters();
void generate_retvals();
void generate_function();

void generate_call();

#endif // _GENERATOR_H