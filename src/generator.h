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
#include "scanner.h"
#include "ibuffer.h"
#include "symtable.h"
#include "func_helper.h"


extern local_symtab_t *local_tab;   // local symtable from parser
extern global_symtab_t *global_tab; // global symtable from parser
extern ibuffer_t *buffer;           // instruction buffer from parser

/**
 * @brief Generate name of identifier
 * @param[in]  local_tab Pointer to local symtable
 * @param[in]  name      Name of variable
 * @param[out] output    Name of variable
 * @return Unique identifier name in format func_name$depth$id_name
 */
void generate_name(string_t *output, string_t name);

void generate_start();
void generate_entry();
void generate_end();
void generate_exit();

void generate_label(string_t label_name);
void generate_parameters();
void generate_retvals();
void generate_function();
void generate_function_end();

void generate_identifier(string_t id_name);

void generate_call_prep(func_def_t *f_helper);
void generate_call_params(token_t *token, func_def_t *f_helper);
//void generate_params_id();
void generate_call(func_def_t *f_helper);

void generate_write(token_t *token);



#endif // _GENERATOR_H