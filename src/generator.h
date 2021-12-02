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
#include "parser_helper.h"
#include "expression.h"
#include "builtin.h"


extern local_symtab_t *local_tab;   // local symtable from parser
extern global_symtab_t *global_tab; // global symtable from parser
extern ibuffer_t *buffer;           // instruction buffer from parser
extern ibuffer_t *defvar_buffer;    // buffer for declaring variables

void generate_name(ibuffer_t *buffer, string_t name);

void generate_start();
void generate_entry();
void generate_end();
void generate_exit();

void generate_label(string_t label_name);
void generate_parameters(parser_helper_t *p_helper);
void generate_retvals();
void generate_function(parser_helper_t *p_helper);
void generate_function_end();

void generate_identifier(ibuffer_t *buffer, string_t id_name);

void generate_call_prep(parser_helper_t *p_helper);
void generate_call_params(token_t *token, parser_helper_t *p_helper);
void generate_call(parser_helper_t *p_helper);
void generate_return_value(int ret_counter);

void generate_write(token_t *token);

void generate_expr_start();
void generate_expr_end();
void generate_push_compare(prec_table_term_t op);
void generate_push_operator(prec_table_term_t op);
void generate_push_operand(token_t *token);

void generate_assign(string_t name);
void generate_assign_function(parser_helper_t *p_helper);

void generate_else();
void generate_if_else();
void generate_if_end();

void generate_while_start();
void generate_while_skip();
void generate_while_end();

void generate_num_conversion(unsigned index);

#endif // _GENERATOR_H
