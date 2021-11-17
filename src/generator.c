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

ibuffer_t *buffer;  // instruction buffer

void generate_start()
{ 
    ADD_INST(".IFJcode21\n");
}

void generate_defvar(char *name)
{
    ADD_INSTOP("DEFVAR ", name);
}

void generate_fdef()
{
    ADD_INST("CREATEFRAME\n");
}
