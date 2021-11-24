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
#include "generator.h"
#include "str.h"

void generate_string(string_t *insert_to, string_t string)
{
    str_insert(insert_to, "string@");

    // special cases for whitespaces, hashtag and backslash
    for (unsigned int i = 0; i < string.length; i++) {
        if (string.str[i] <= ' ') {
            str_insert(insert_to, "\\0");
            str_insert_int(insert_to, string.str[i]);
        } else if (string.str[i] == '#') {
            str_insert(insert_to, "\\0");
            str_insert_int(insert_to, string.str[i]);
        } else if (string.str[i] == '\\') {
            str_insert(insert_to, "\\0");
            str_insert_int(insert_to, string.str[i]);
        } else {
            str_add_char(insert_to, string.str[i]);
        }
    }
}

void generate_int(string_t *insert_to, int number)
{
    str_insert(insert_to, "int@");
    str_insert_int(insert_to, number);
}

void generate_decimal(string_t *insert_to, double number)
{
    str_insert(insert_to, "float@");
    str_insert_double(insert_to, number);
}

void generate_name(string_t *output, string_t name)
{
    local_symtab_t *symtab = local_symtab_find(local_tab, name);
    if (symtab == NULL)
        return;

    str_insert(output, local_tab->key.str);
    str_insert(output, "$");
    str_insert_int(output, symtab->depth);
    str_insert(output, "$");
    str_insert(output, name.str);
}

void generate_entry()
{
    ADD_NEWLINE();
    ADD_INST("label _start_");
    ADD_NEWLINE();
}

void generate_start()
{
    ADD_INST_N(".IFJcode21");

    // global variable to store results of comparisons in expressions
    ADD_INST_N("defvar GF@bool"); 

    // global variables to store operands of advanced comparisons in expressions 
    ADD_INST_N("defvar GF@adv_comp1");
    ADD_INST_N("defvar GF@adv_comp2");

    ADD_INST_N("jump _start_");
}

void generate_end()
{
    ADD_INST_N("jump _end_");
}

void generate_exit()
{
    ADD_INST_N("label _end_");
}

void generate_label(string_t label_name)
{
    ADD_NEWLINE();
    ADD_INST("label ");
    strcat(INST, label_name.str);
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
        ADD_INST("defvar LF@%retval");
        str_insert_int(&retval_num, retval);
        strcat(INST, retval_num.str);
        ADD_NEWLINE();

        // initialize its value to nil
        ADD_INST("move LF@%retval");
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
        // variable definition
        ADD_INST("defvar LF@");
        // create unique identificator name
        generate_name(&id_mangled, local_tab->data[i]->name);
        strcat(INST, id_mangled.str);
        ADD_NEWLINE();

        // assign value from function call
        ADD_INST("move LF@");
        strcat(INST, id_mangled.str);
        strcat(INST, " LF@%");
        // id_mangled is now used to convert iterator variable into string
        str_clear(&id_mangled);
        str_insert_int(&id_mangled, i);
        strcat(INST, id_mangled.str);
        ADD_NEWLINE();

        str_clear(&id_mangled);
        ADD_NEWLINE();
    }

    str_free(&id_mangled);
}

void generate_function()
{
    ADD_NEWLINE();
    generate_label(local_tab->key);

    // push previously set up temporary frame to frame stack
    // (TF@var becomes LF@var)
    ADD_INST("pushframe");
    ADD_NEWLINE();

    generate_retvals();
    ADD_NEWLINE();
    generate_parameters();
}

void generate_function_end()
{
    ADD_INST("popframe");
    ADD_NEWLINE();
    ADD_INST("return");
    ADD_NEWLINE();
}

void generate_identifier(string_t id_name)
{
    string_t id_mangled;
    str_init(&id_mangled);

    generate_name(&id_mangled, id_name);
    ADD_INST("defvar LF@");
    strcat(INST, id_mangled.str);
    ADD_NEWLINE();

    str_free(&id_mangled);
}

void generate_call_prep(func_def_t *f_helper)
{
    ADD_INST("createframe");
    ADD_NEWLINE();

    string_t param_name;
    str_init(&param_name);
    // generate generic names for function parameters
    for (int i = 0; i < str_len(f_helper->item->params); i++) {
        ADD_INST("defvar TF@");
        str_insert(&param_name, "%");
        str_insert_int(&param_name, i);
        strcat(INST, param_name.str);
        ADD_NEWLINE();
        str_clear(&param_name);
    }
    str_free(&param_name);
}

void generate_call_params(token_t *token, func_def_t *f_helper)
{
    ADD_INST("move TF@");

    string_t param_name;
    str_init(&param_name);

    str_insert(&param_name, "%");
    str_insert_int(&param_name, f_helper->par_counter);
    str_add_char(&param_name, ' ');
    f_helper->par_counter++;

    switch (token->type)
    {
    case TOK_STRING:
        generate_string(&param_name, token->attribute.s);
        break;

    case TOK_INT:
        generate_int(&param_name, token->attribute.number);
        break;

    case TOK_DECIMAL:
        generate_decimal(&param_name, token->attribute.decimal);
        break;

    case TOK_ID:
        str_insert(&param_name, "LF@");
        generate_name(&param_name, token->attribute.s);
        break;

    default:
        break;
    }

    strcat(INST, param_name.str);
    ADD_NEWLINE();
    str_free(&param_name);
}

void generate_call(func_def_t *f_helper)
{
    ADD_INST("call ");
    strcat(INST, f_helper->item->key.str);
    ADD_NEWLINE();
}

void generate_write(token_t *token)
{
    string_t value;
    str_init(&value);

    ADD_INST("write ");

    switch (token->type)
    {
    case TOK_STRING:
        generate_string(&value, token->attribute.s);
        break;

    case TOK_INT:
        generate_int(&value, token->attribute.number);
        break;

    case TOK_DECIMAL:
        generate_decimal(&value, token->attribute.decimal);
        break;

    case TOK_ID:
        str_insert(&value, "LF@");
        generate_name(&value, token->attribute.s);

    default:
        break;
    }

    strcat(INST, value.str);
    ADD_NEWLINE();

    str_free(&value);
}

void generate_assign(string_t name)
{
    string_t id_name;
    str_init(&id_name);

    // pop instruction to variable
    ADD_INST("pops ");
    str_insert(&id_name, "LF@");
    generate_name(&id_name, name);
    strcat(INST, id_name.str);
    ADD_NEWLINE();

    str_free(&id_name);
}

void generate_if_label(string_t *insert_to, char *label_or_jump)
{
    str_insert(insert_to, label_or_jump);
    str_add_char(insert_to, '_');
    str_insert(insert_to, local_tab->key.str);
    str_add_char(insert_to, '_');
    str_insert_int(insert_to, local_tab->if_cnt);
}

void generate_else()
{
    string_t label_name;
    str_init(&label_name);

    // if cond is true, skip else part
    generate_if_label(&label_name, "jump ");
    str_insert(&label_name, "_end");
    strcat(INST, label_name.str);
    ADD_NEWLINE();
    str_clear(&label_name);

    generate_if_label(&label_name, "label ");
    str_insert(&label_name, "_else");
    strcat(INST, label_name.str);
    ADD_NEWLINE();
    
    str_free(&label_name);
}

void generate_if_end()
{
    string_t label_name;
    str_init(&label_name);

    generate_if_label(&label_name, "label ");
    str_insert(&label_name, "_end");
    strcat(INST, label_name.str);
    ADD_NEWLINE();
    
    str_free(&label_name);
}


void generate_push_compare(prec_table_term_t op)
{
    switch (op)
    {
    case EQ:
        ADD_INST_N("eqs");
        break;
    
    case NOT_EQ:
        ADD_INST_N("eqs");
        ADD_INST_N("nots");
        break;

    case LESS:
        ADD_INST_N("lts");
        break;

    case LESS_EQ:
        // store variables
        ADD_INST_N("pops GF@adv_comp2");
        ADD_INST_N("pops GF@adv_comp1");
        
        // compare < only
        ADD_INST_N("pushs GF@adv_comp1");
        ADD_INST_N("pushs GF@adv_comp2");
        ADD_INST_N("lts");

        // compare ==
        ADD_INST_N("pushs GF@adv_comp1");
        ADD_INST_N("pushs GF@adv_comp2");
        ADD_INST_N("eqs");

        // compare <=
        ADD_INST_N("ors");
        break;
        
    case GREAT:
        ADD_INST_N("gts");
        break;

    case GREAT_EQ:
        // store variables
        ADD_INST_N("pops GF@adv_comp2");
        ADD_INST_N("pops GF@adv_comp1");
        
        // compare > only
        ADD_INST_N("pushs GF@adv_comp1");
        ADD_INST_N("pushs GF@adv_comp2");
        ADD_INST_N("gts");

        // compare ==
        ADD_INST_N("pushs GF@adv_comp1");
        ADD_INST_N("pushs GF@adv_comp2");
        ADD_INST_N("eqs");

        // compare >=
        ADD_INST_N("ors");
        break;

    default:
        break;
    }

    ADD_INST_N("pops GF@bool");

    string_t label_name;
    str_init(&label_name);

    generate_if_label(&label_name, "jumpifneq ");
    str_insert(&label_name, "_else GF@bool bool@true");
    strcat(INST, label_name.str);
    ADD_NEWLINE();

    str_free(&label_name);
}

void generate_push_arithmetic(prec_table_term_t op)
{
    switch (op)
    {
    case MINUS:
        ADD_INST_N("subs");
        break;
    
    case PLUS:
        ADD_INST_N("adds");
        break;

    case MUL:
        ADD_INST_N("muls");
        break;

    case DIV:
        ADD_INST_N("divs");
        break;
        
    case DIV_INT:
        ADD_INST_N("idivs");
        break;

    default:
        break;
    }
}

void generate_push_operator(prec_table_term_t op)
{
    switch (op)
    {
    case MINUS:
    case PLUS:
    case MUL:
    case DIV:
    case DIV_INT:
        generate_push_arithmetic(op);
        break;
    
    case EQ:
    case NOT_EQ:
    case LESS:
    case LESS_EQ:
    case GREAT:
    case GREAT_EQ:
        generate_push_compare(op);
        break;

    default:
        break;
    }
}

void generate_push_operand(token_t *token)
{
    string_t name;
    str_init(&name);

    ADD_INST("pushs ");

    switch (token->type)
    {
    case TOK_STRING:
        generate_string(&name, token->attribute.s);
        break;
        
    case TOK_DECIMAL:
        generate_decimal(&name, token->attribute.decimal);
        break;

    case TOK_INT:
        generate_int(&name, token->attribute.number);
        break;

    case TOK_ID:
        str_insert(&name, "LF@");
        generate_name(&name, token->attribute.s);
        break;
    
    default:
        break;
    }

    strcat(INST, name.str);
    ADD_NEWLINE();

    str_free(&name);
}