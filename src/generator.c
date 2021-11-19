/**
 * VUT IFJ Project 2021.
 *
 * @file generator.c
 * 
 * @brief Implementation of code generation (stack code)
 * 
 * @author Vojtěch Eichler 
 * @author Václav Korvas
 * @author Tomáš Matuš
 * @author Adam Zvara
 */

#include <stdio.h>
#include <string.h>
#include "generator.h"
#include "ibuffer.h"
#include "str.h"

void generate_name(local_symtab_t *local_tab, string_t name, string_t *output)
{
    str_insert(output, local_tab->key.str);
    str_insert(output, "$");
    str_insert_int(output, local_tab->depth);
    str_insert(output, "$");
    str_insert(output, name.str);
}

void generate_start()
{ 
    ADD_INST(".IFJcode21");
    ADD_NEWLINE();
}

void generate_label(string_t name)
{
    ADD_INST("label ");
    strcat(INST, name.str);
    ADD_NEWLINE();
}

void generate_retvals()
{
    // storage for converting number to string
    string_t retval_num;
    str_init(&retval_num);
    
    // item in global symtable corresponding to function
    struct global_item *func = global_find(global_tab, local_tab->key);

    // create local variables for return values in format LF@retval%N
    // N is the position of return value    
    for (int retval = 0; retval < str_len(func->retvals); retval++) {
        // define variable
        ADD_INST("defvar LF@retval%");
        str_insert_int(&retval_num, retval);
        strcat(INST, retval_num.str);
        ADD_NEWLINE();

        // initialize its value to nil
        ADD_INST("move LF@retval%");
        strcat(INST, retval_num.str);
        strcat(INST, " nil@nil");

        str_clear(&retval_num);
    }

    str_free(&retval_num);
}

void generate_parameters()
{
    // define formal parameters of function
    string_t id_mangled;
    str_init(&id_mangled);

    // iterate through all identifiers
    // (at this point, only parameters are in local symtable)
    for (unsigned int i = 0; i < local_tab->size; i++) {
        ADD_INST("defvar LF@");

        // create unique identificator name 
        generate_name(local_tab, local_tab->data[i]->name, &id_mangled);
        strcat(INST, id_mangled.str);

        ADD_NEWLINE();

        str_clear(&id_mangled);
    }

    str_free(&id_mangled);
}

void generate_function()
{
    ADD_NEWLINE(); // just so this part is separated from the previous

    generate_label(local_tab->key);

    // push previously set up temporary frame to frame stack
    // (TF@var becomes LF@var)
    ADD_INST("pushframe");
    ADD_NEWLINE();

    generate_retvals();
    generate_parameters();
}

void generate_call()
{
    
}