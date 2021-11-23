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

#define ADD_INST_N(string)  \
do {                        \
    ADD_INST(string);       \
    ADD_NEWLINE();          \
} while (0)                 \

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
    ADD_INST_N("label reads");
    ADD_INST_N("pushframe");
    ADD_INST_N("defvar LF@%retval0");
    ADD_INST_N("read LF@%retval0 string");
    ADD_INST_N("popframe");
    ADD_INST_N("return");
}

void generate_readi()
{
    ADD_NEWLINE();
    ADD_INST_N("label readi");
    ADD_INST_N("pushframe");
    ADD_INST_N("defvar LF@%retval0");
    ADD_INST_N("read LF@%retval0 int");
    ADD_INST_N("popframe");
    ADD_INST_N("return");
}

void generate_readn()
{
    ADD_NEWLINE();
    ADD_INST_N("label readn");
    ADD_INST_N("pushframe");
    ADD_INST_N("defvar LF@%retval0");
    ADD_INST_N("read LF@%retval0 float");
    ADD_INST_N("popframe");
    ADD_INST_N("return");
}

void generate_chr()
{
    ADD_NEWLINE();
    ADD_INST_N("label chr");
    ADD_INST_N("pushframe");
    ADD_INST_N("defvar LF@%retval0");
    ADD_INST_N("move LF@%retval0 nil@nil");

    // variable to chceck if parameter is not nil
    ADD_INST_N("defvar LF@%param_type");
    ADD_INST_N("type LF@%param_type LF@%0");

    ADD_INST_N("jumpifeq _not_nil LF@%param_type string@int");
    ADD_INST_N("exit int@8");

    // retval is not nil
    ADD_INST_N("label _not_nil");
    ADD_INST_N("defvar LF@%bool");

    // 0 < i < 255
    ADD_INST_N("lt LF@%bool LF@%0 int@0");
    ADD_INST_N("jumpifeq _chr_end LF@%bool bool@true");
    ADD_INST_N("gt LF@%bool LF@%0 int@255");
    ADD_INST_N("jumpifeq _chr_end LF@%bool bool@true");

    ADD_INST_N("int2char LF@%retval0 LF@%0");
    ADD_INST_N("write LF@%retval0");

    ADD_INST_N("label _chr_end");
    ADD_INST_N("popframe");
    ADD_INST_N("return");
}

void generate_ord()
{
    ADD_NEWLINE();
    ADD_INST_N("label ord");
    ADD_INST_N("pushframe");
    ADD_INST_N("defvar LF@%retval0");
    ADD_INST_N("move LF@%retval0 nil@nil");

    // check if parameters are not nil
    ADD_INST_N("defvar LF@%param_type");
    ADD_INST_N("type LF@%param_type LF@%0");
    ADD_INST_N("jumpifeq _ord_par2 LF@%param_type string@string");
    ADD_INST_N("exit int@8");

    ADD_INST_N("label _ord_par2");
    ADD_INST_N("type LF@%param_type LF@%1");
    ADD_INST_N("jumpifeq _ord_cont LF@%param_type string@int");
    ADD_INST_N("exit int@8");

    // parameters are not nil
    ADD_INST_N("label _ord_cont");
    ADD_INST_N("defvar LF@%strlen");
    ADD_INST_N("strlen LF@%strlen LF@%0");
    ADD_INST_N("sub LF@%strlen LF@%strlen int@1");

    // check if index is in range 0 < i - 1 < strlen - 1
    ADD_INST_N("sub LF@%1 LF@%1 int@1");
    ADD_INST_N("defvar LF@%bool");
    ADD_INST_N("lt LF@%bool LF@%1 int@0");
    ADD_INST_N("jumpifeq _ord_end LF@%bool bool@true");
    ADD_INST_N("gt LF@%bool LF@%1 LF@%strlen");
    ADD_INST_N("jumpifeq _ord_end LF@%bool bool@true");

    ADD_INST_N("stri2int LF@%retval0 LF@%0 LF@%1");
    ADD_INST_N("write LF@%retval0");

    ADD_INST_N("label _ord_end");
    ADD_INST_N("popframe");
    ADD_INST_N("return");
}

void generate_substr()
{
    ADD_NEWLINE();
    ADD_INST_N("label substr");
    ADD_INST_N("pushframe");
    ADD_INST_N("defvar LF@%substring");
    ADD_INST_N("defvar LF@%iterator");
    ADD_INST_N("defvar LF@%begin");
    ADD_INST_N("defvar LF@%end");
    ADD_INST_N("defvar LF@%char");

    // check if any param is nil
    ADD_INST_N("defvar LF@%param_type");
    ADD_INST_N("type LF@%param_type LF@%0");
    ADD_INST_N("jumpifeq _substr_par2 LF@%param_type string@string");
    ADD_INST_N("exit int@8");

    ADD_INST_N("label _substr_par2");
    ADD_INST_N("type LF@%param_type LF@%1");
    ADD_INST_N("jumpifeq _substr_par3 LF@%param_type string@float");
    ADD_INST_N("exit int@8");

    ADD_INST_N("label _substr_par3");
    ADD_INST_N("type LF@%param_type LF@%2");
    ADD_INST_N("jumpifeq _substr_cont LF@%param_type string@float");
    ADD_INST_N("exit int@8");


    ADD_INST_N("label _substr_cont");

    // initialize empty substring
    ADD_INST_N("move LF@%substring string@");

    // convert given numbers into integers
    ADD_INST_N("float2int LF@%begin LF@%1");
    ADD_INST_N("float2int LF@%end LF@%2");

    // substract 1 from begin, since getchar uses indexes from 0
    ADD_INST_N("sub LF@%begin LF@%begin int@1");

    ADD_INST_N("move LF@%iterator LF@%begin");

    // if i > j return empty string
    ADD_INST_N("defvar LF@%bool");
    ADD_INST_N("gt LF@%bool LF@%iterator LF@%end");
    ADD_INST_N("jumpifeq _substr_end LF@%bool bool@true");

    ADD_INST_N("defvar LF@%strlen");
    ADD_INST_N("strlen LF@%strlen LF@%0");

    // 1 < j < strlen
    ADD_INST_N("lt LF@%bool LF@%end int@1");
    ADD_INST_N("jumpifeq _substr_end LF@%bool bool@true");
    ADD_INST_N("gt LF@%bool LF@%end LF@%strlen");
    ADD_INST_N("jumpifeq _substr_end LF@%bool bool@true");

    // 0 < i - 1 < strlen - 1
    ADD_INST_N("sub LF@%strlen LF@%strlen int@1");
    ADD_INST_N("lt LF@%bool LF@%begin int@0");
    ADD_INST_N("jumpifeq _substr_end LF@%bool bool@true");
    ADD_INST_N("gt LF@%bool LF@%begin LF@%strlen");
    ADD_INST_N("jumpifeq _substr_end LF@%bool bool@true");

    ADD_INST_N("label _substr_loop");  //loop start

    // if iterator == j, jump to end
    ADD_INST_N("jumpifeq _substr_end LF@%iterator LF@%end");

    // get single char from string and store it in LF@char
    ADD_INST_N("getchar LF@%char LF@%0 LF@%iterator");

    // concatenate substring with newly extracted char
    ADD_INST_N("concat LF@%substring LF@%substring LF@%char");

    ADD_INST_N("add LF@%iterator LF@%iterator int@1");
    ADD_INST_N("jump _substr_loop");
    // TODO: check if string is OK, index in range

    ADD_INST_N("label _substr_end");
    ADD_INST_N("write LF@%substring");

    ADD_INST_N("popframe");
    ADD_INST_N("return");
}

void generate_tointeger()
{
    ADD_NEWLINE();
    ADD_INST_N("label tointeger");
    ADD_INST_N("pushframe");
    ADD_INST_N("defvar LF@%retval0");
    ADD_INST_N("move LF@%retval0 nil@nil");

    // variable to chceck if param_type is not nil
    ADD_INST_N("defvar LF@%param_type");
    ADD_INST_N("type LF@%param_type LF@%0");

    ADD_INST_N("jumpifeq _tointeger_cont LF@%param_type string@float");
    ADD_INST_N("jump _tointeger_end");

    // retval is not nil
    ADD_INST_N("label _tointeger_cont");
    ADD_INST_N("float2int LF@%retval0 LF@%0");

    ADD_INST_N("label _tointeger_end");
    ADD_INST_N("popframe");
    ADD_INST_N("return");
}