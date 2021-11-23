/**
 * VUT IFJ Project 2021.
 *
 * @file builtin.c
 *
 * @brief Builtin functions for IFJ21
 *
 * @author Vojtěch Eichler
 * @author Václav Korvas
 * @author Tomáš Matuš
 * @author Adam Zvara
 */

#include <string.h>
#include "builtin.h"
#include "generator.h"
#include "ibuffer.h"
#include "str.h"

extern ibuffer_t *buffer;           // instruction buffer from parser

builtin_used_t *builtin_used_create()
{
    builtin_used_t *bu = malloc(sizeof(*bu));
    if (bu == NULL) {
        return NULL;
    }

    bu->reads = false;
    bu->readn = false;
    bu->readi = false;
    bu->tointeger = false;
    bu->substr = false;
    bu->ord = false;
    bu->chr = false;

    return bu;
}

void builtin_used_update(builtin_used_t *bu, string_t name)
{
    if (!strcmp(name.str, "reads")) {
        bu->reads = true;
    } else if (!strcmp(name.str, "readn")) {
        bu->readn = true;
    } else if (!strcmp(name.str, "readi")) {
        bu->readi = true;
    } else if (!strcmp(name.str, "tointeger")) {
        bu->tointeger = true;
    } else if (!strcmp(name.str, "substr")) {
        bu->substr = true;
    } else if (!strcmp(name.str, "ord")) {
        bu->ord = true;
    } else if (!strcmp(name.str, "chr")) {
        bu->chr = true;
    }
}

void builtin_used_generate(builtin_used_t *bu)
{
    if (bu->reads) {
        generate_reads();
    }
    if (bu->readn) {
        generate_readn();
    }
    if (bu->readi) {
        generate_readi();
    }
    if (bu->tointeger) {
        generate_tointeger();
    }
    if (bu->substr) {
        generate_substr();
    }
    if (bu->ord) {
        generate_ord();
    }
    if (bu->chr) {
        generate_chr();
    }
}

void add_builtin(global_symtab_t *gs)
{
    string_t function_name;
    str_init(&function_name);

    // reads
    str_insert(&function_name, "reads");
    struct global_item *func = global_add(gs, function_name);
    func->defined = true;
    str_add_char(&func->retvals, 's');
    str_clear(&function_name);

    // readi
    str_insert(&function_name, "readi");
    func = global_add(gs, function_name);
    func->defined = true;
    str_add_char(&func->retvals, 'i');
    str_clear(&function_name);

    // readn
    str_insert(&function_name, "readn");
    func = global_add(gs, function_name);
    func->defined = true;
    str_add_char(&func->retvals, 'n');
    str_clear(&function_name);

    // write
    str_insert(&function_name, "write");
    func = global_add(gs, function_name);
    func->defined = true;
    str_clear(&function_name);

    // tointeger
    str_insert(&function_name, "tointeger");
    func = global_add(gs, function_name);
    func->defined = true;
    str_add_char(&func->retvals, 'i');
    str_add_char(&func->params, 'n');
    str_clear(&function_name);

    // substr
    str_insert(&function_name, "substr");
    func = global_add(gs, function_name);
    func->defined = true;
    str_add_char(&func->retvals, 's');
    str_insert(&func->params, "snn");
    str_clear(&function_name);

    // ord
    str_insert(&function_name, "ord");
    func = global_add(gs, function_name);
    func->defined = true;
    str_add_char(&func->retvals, 'i');
    str_insert(&func->params, "si");
    str_clear(&function_name);

    // chr
    str_insert(&function_name, "chr");
    func = global_add(gs, function_name);
    func->defined = true;
    str_add_char(&func->retvals, 's');
    str_add_char(&func->params, 'i');
    str_clear(&function_name);
}

void generate_reads()
{
    ADD_NEWLINE();
    ADD_INST("label reads");
    ADD_NEWLINE();
    ADD_INST("pushframe");
    ADD_NEWLINE();
    ADD_INST("defvar LF@%retval0");
    ADD_NEWLINE();
    ADD_INST("read LF@%retval0 string");
    ADD_NEWLINE();
    ADD_INST("popframe");
    ADD_NEWLINE();
    ADD_INST("return");
    ADD_NEWLINE();
}

void generate_readi()
{
    ADD_NEWLINE();
    ADD_INST("label readi");
    ADD_NEWLINE();
    ADD_INST("pushframe");
    ADD_NEWLINE();
    ADD_INST("defvar LF@%retval0");
    ADD_NEWLINE();
    ADD_INST("read LF@%retval0 int");
    ADD_NEWLINE();
    ADD_INST("popframe");
    ADD_NEWLINE();
    ADD_INST("return");
    ADD_NEWLINE();
}

void generate_readn()
{
    ADD_NEWLINE();
    ADD_INST("label readn");
    ADD_NEWLINE();
    ADD_INST("pushframe");
    ADD_NEWLINE();
    ADD_INST("defvar LF@%retval0");
    ADD_NEWLINE();
    ADD_INST("read LF@%retval0 float");
    ADD_NEWLINE();
    ADD_INST("popframe");
    ADD_NEWLINE();
    ADD_INST("return");
    ADD_NEWLINE();
}

void generate_chr()
{
    ADD_NEWLINE();
    ADD_INST("label chr");
    ADD_NEWLINE();
    ADD_INST("pushframe");
    ADD_NEWLINE();
    ADD_INST("defvar LF@%retval0");
    ADD_NEWLINE();

    // TODO: check if number is in range, if number is not nil

    ADD_INST("int2char LF@%retval0 LF@%0");
    ADD_NEWLINE();
    ADD_INST("write LF@%retval0");
    ADD_NEWLINE();

    ADD_INST("popframe");
    ADD_NEWLINE();
    ADD_INST("return");
    ADD_NEWLINE();
}

void generate_ord()
{
    ADD_NEWLINE();
    ADD_INST("label ord");
    ADD_NEWLINE();
    ADD_INST("pushframe");
    ADD_NEWLINE();
    ADD_INST("defvar LF@%retval0");
    ADD_NEWLINE();

    // TODO: check if string is OK, index in range

    ADD_INST("stri2int LF@%retval0 LF@%0 LF@%1");
    ADD_NEWLINE();
    ADD_INST("write LF@%retval0");
    ADD_NEWLINE();

    ADD_INST("popframe");
    ADD_NEWLINE();
    ADD_INST("return");
    ADD_NEWLINE();
}

void generate_substr()
{
    ADD_NEWLINE();
    ADD_INST("label substr");
    ADD_NEWLINE();
    ADD_INST("pushframe");
    ADD_NEWLINE();
    ADD_INST("defvar LF@substring");
    ADD_NEWLINE();
    ADD_INST("defvar LF@iterator");
    ADD_NEWLINE();
    ADD_INST("defvar LF@begin");
    ADD_NEWLINE();
    ADD_INST("defvar LF@end");
    ADD_NEWLINE();
    ADD_INST("defvar LF@char");
    ADD_NEWLINE();

    // initialize empty substring
    ADD_INST("move LF@substring string@");
    ADD_NEWLINE();

    // convert given numbers into integers
    ADD_INST("float2int LF@begin LF@%1");
    ADD_NEWLINE();
    ADD_INST("float2int LF@end LF@%2");
    ADD_NEWLINE();

    // substract 1 from begin, since getchar uses indexes from 0
    ADD_INST("sub LF@begin LF@begin int@1");
    ADD_NEWLINE();

    ADD_INST("move LF@iterator LF@begin");
    ADD_NEWLINE();

    ADD_INST("label _substr_loop");  //loop start
    ADD_NEWLINE();

    // if iterator == j, jump to end
    ADD_INST("jumpifeq _substr_end LF@iterator LF@end");
    ADD_NEWLINE();

    // get single char from string and store it in LF@char
    ADD_INST("getchar LF@char LF@%0 LF@iterator");
    ADD_NEWLINE();

    // concatenate substring with newly extracted char
    ADD_INST("concat LF@substring LF@substring LF@char");
    ADD_NEWLINE();

    ADD_INST("add LF@iterator LF@iterator int@1");
    ADD_NEWLINE();
    ADD_INST("jump _substr_loop");
    ADD_NEWLINE();
    // TODO: check if string is OK, index in range

    ADD_INST("label _substr_end");
    ADD_NEWLINE();
    ADD_INST("write LF@substring");
    ADD_NEWLINE();

    ADD_INST("popframe");
    ADD_NEWLINE();
    ADD_INST("return");
    ADD_NEWLINE();
}

void generate_tointeger()
{
    ADD_NEWLINE();
    ADD_INST("label tointeger");
    ADD_NEWLINE();
    ADD_INST("pushframe");
    ADD_NEWLINE();
    ADD_INST("defvar LF@%retval0");
    ADD_NEWLINE();
    ADD_INST("float2int LF@%retval0 LF@%0");
    ADD_NEWLINE();
    ADD_INST("write LF@%retval0");
    ADD_NEWLINE();
    ADD_INST("popframe");
    ADD_NEWLINE();
    ADD_INST("return");
    ADD_NEWLINE();
}