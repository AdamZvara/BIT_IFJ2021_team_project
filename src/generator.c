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

/* functions for converting constants into IFJcode21 constants */
void generate_string(string_t string)
{
    string_t generated;
    str_init(&generated);
    str_insert(&generated, "string@");

    // special cases for whitespaces, hashtag and backslash
    for (unsigned int i = 0; i < string.length; i++) {
        if (string.str[i] <= ' ') {
            str_insert(&generated, "\\0");
            if (string.str[i] < 10) {
                str_add_char(&generated, '0');
            }
            str_insert_int(&generated, string.str[i]);
        } else if (string.str[i] == '#') {
            str_insert(&generated, "\\0");
            str_insert_int(&generated, string.str[i]);
        } else if (string.str[i] == '\\') {
            str_insert(&generated, "\\0");
            str_insert_int(&generated, string.str[i]);
        } else {
            str_add_char(&generated, string.str[i]);
        }
    }

    strcat(INST, generated.str);
    str_free(&generated);
}

void generate_int(int number)
{
    string_t generated;
    str_init(&generated);

    str_insert(&generated, "int@");
    str_insert_int(&generated, number);

    strcat(INST, generated.str);
    str_free(&generated);
}

void generate_decimal(double number)
{
    string_t generated;
    str_init(&generated);

    str_insert(&generated, "float@");
    str_insert_double(&generated, number);

    strcat(INST, generated.str);
    str_free(&generated);
}
/*             END IFJCODE21 constants                    */


// Function to create mangled names of identifiers in function
void generate_name(ibuffer_t *buffer, string_t name)
{
    local_symtab_t *symtab = local_symtab_find(local_tab, name);
    if (symtab == NULL)
        return;

    string_t generated;
    str_init(&generated);

    str_insert(&generated, local_tab->key.str);
    str_insert(&generated, "$");
    str_insert_int(&generated, symtab->depth);
    str_insert(&generated, "$");
    str_insert(&generated, name.str);

    strcat(INST, generated.str);
    str_free(&generated);
}

/* Functions to generate entry points/ exit points of program */

// generate prolog, global variables, jump to entry point
void generate_start()
{
    ADD_INST_N(".IFJcode21");

    // global variable to store results of comparisons in expressions
    ADD_INST_N("defvar GF@bool");

    // global variables to store operands of advanced comparisons/strlen/concat in expressions
    ADD_INST_N("defvar GF@arg1");
    ADD_INST_N("defvar GF@arg2");
    ADD_INST_N("defvar GF@output");

    ADD_INST_N("jump _start_");
}

// generate entry point - first call of function in main body of program
void generate_entry()
{
    ADD_NEWLINE();
    ADD_INST_N("label _start_");
}

void generate_end()
{
    ADD_INST_N("jump _end_");
}

void generate_div_by_zero()
{
    ADD_INST_N("label _div_by_zero");
    ADD_INST_N("exit int@9");
}

void generate_nil_with_operator()
{
    ADD_INST_N("label _nil_with_operator");
    ADD_INST_N("exit int@8");
}

void generate_write_nil()
{
    ADD_INST_N("label _write_nil");
    ADD_INST_N("write string@nil");
    ADD_INST_N("return");
}

void generate_exit()
{
    ADD_INST_N("label _end_");
}
/*            END IFJcode21 ENTRY                  */

// generate label from given string
void generate_label(string_t label_name)
{
    ADD_NEWLINE();
    ADD_INST("label ");
    strcat(INST, label_name.str);
    ADD_NEWLINE();
}

/*           FUNCTION ENTRY               */
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
        ADD_NEWLINE();

        str_clear(&retval_num);
    }

    str_free(&retval_num);
}

void generate_parameters(parser_helper_t *p_helper)
{
    string_t num;
    str_init(&num);

    int par_cnt = 0;

    // iterate through all identifiers in p_helper and create function parameters
    struct identifiers *tmp = p_helper->id_first;
    while (tmp != NULL) {
        // variable definition
        ADD_INST("defvar LF@");
        // create unique identificator name
        generate_name(buffer, tmp->data->name);
        ADD_NEWLINE();

        // assign value from function call
        ADD_INST("move LF@");
        generate_name(buffer, tmp->data->name);
        strcat(INST, " LF@%");

        str_insert_int(&num, par_cnt++);
        strcat(INST, num.str);
        ADD_NEWLINE();

        str_clear(&num);
        ADD_NEWLINE();
        tmp = tmp->next;
    }

    str_free(&num);
}

void generate_function(parser_helper_t *p_helper)
{
    ADD_NEWLINE();
    generate_label(local_tab->key);

    // push previously set up temporary frame to frame stack
    // (TF@var becomes LF@var)
    ADD_INST_N("pushframe");

    generate_retvals();
    ADD_NEWLINE();
    generate_parameters(p_helper);
}

void generate_function_end()
{
    ADD_INST_N("popframe");
    ADD_INST_N("return");
}

/*          END FUNCTION ENTRY             */

// generate local identifers with mangled name
void generate_identifier(ibuffer_t *buffer, string_t id_name)
{
    ADD_INST("defvar LF@");
    generate_name(buffer, id_name);
    ADD_NEWLINE();
    ADD_INST("move LF@");
    generate_name(buffer, id_name);
    strcat(INST, " nil@nil");
    ADD_NEWLINE();
}

/*          FUNCTION CALL          */
void generate_call_prep(parser_helper_t *p_helper)
{
    ADD_INST("createframe");
    ADD_NEWLINE();

    string_t param_name;
    str_init(&param_name);
    // generate generic names for function parameters
    for (int i = 0; i < str_len(p_helper->func->params); i++) {
        ADD_INST("defvar TF@");
        str_insert(&param_name, "%");
        str_insert_int(&param_name, i);
        strcat(INST, param_name.str);
        ADD_NEWLINE();
        str_clear(&param_name);
    }
    str_free(&param_name);
}

void generate_call_params(token_t *token, parser_helper_t *p_helper)
{
    ADD_INST("move TF@");

    string_t param_name;
    str_init(&param_name);

    str_insert(&param_name, "%");
    str_insert_int(&param_name, p_helper->par_counter);
    str_add_char(&param_name, ' ');
    p_helper->par_counter++;
    strcat(INST, param_name.str);

    switch (token->type)
    {
    case TOK_STRING:
        generate_string(token->attribute.s);
        break;

    case TOK_INT:
        generate_int(token->attribute.number);
        break;

    case TOK_DECIMAL:
        generate_decimal(token->attribute.decimal);
        break;

    case TOK_ID:
        strcat(INST, "LF@");
        generate_name(buffer, token->attribute.s);
        break;

    default:
        break;
    }

    ADD_NEWLINE();
    str_free(&param_name);
}

void generate_call(parser_helper_t *p_helper)
{
    ADD_INST("call ");
    strcat(INST, p_helper->func->key.str);
    ADD_NEWLINE();
}

void generate_return_value(int ret_counter)
{
    string_t retval_num;
    str_init(&retval_num);

    ADD_INST("pops LF@%retval");
    str_insert_int(&retval_num, ret_counter);
    strcat(INST, retval_num.str);
    ADD_NEWLINE();

    str_free(&retval_num);
}

/*          END FUNCTION CALL             */

// builtin write function
void generate_write(token_t *token)
{
    static int counter = 0;

    switch (token->type)
    {
    case TOK_STRING:
        ADD_INST("write ");
        generate_string(token->attribute.s);
        break;

    case TOK_INT:
        ADD_INST("write ");
        generate_int(token->attribute.number);
        break;

    case TOK_DECIMAL:
        ADD_INST("write ");
        generate_decimal(token->attribute.decimal);
        break;

    case TOK_ID:
        // test if variable is nil
        ADD_INST("type GF@bool ");
        strcat(INST, "LF@");
        generate_name(buffer, token->attribute.s);
        ADD_NEWLINE();

        string_t label_name;
        str_init(&label_name);

        str_insert(&label_name, "_write_not_nil");
        str_insert_int(&label_name, counter);

        ADD_INST("jumpifneq ");
        strcat(INST, label_name.str);
        strcat(INST, " GF@bool string@nil");
        ADD_NEWLINE();

        ADD_INST_N("call _write_nil");

        ADD_INST("label ");
        strcat(INST, label_name.str);
        ADD_NEWLINE();

        ADD_INST("write ");
        strcat(INST, "LF@");
        generate_name(buffer, token->attribute.s);

        counter++;
    default:
        break;
    }

    ADD_NEWLINE();
}

// single assign with expression
void generate_assign(string_t name)
{
    // pop instruction to variable
    ADD_INST("pops LF@");
    generate_name(buffer, name);
    ADD_NEWLINE();
}

// assign function return values to identifiers
void generate_assign_function(parser_helper_t *p_helper)
{
    // counter for retvals
    int counter = 0;
    string_t counter_string;
    str_init(&counter_string);

    // iterate through identifiers and move value from retval into ID
    struct identifiers *tmp = p_helper->id_first;
    while (tmp != NULL) {
        ADD_INST("move LF@");
        generate_name(buffer, tmp->data->name);
        strcat(INST, " TF@%retval");
        str_insert_int(&counter_string, counter);
        counter++;
        strcat(INST, counter_string.str);
        ADD_NEWLINE();
        tmp = tmp->next;
        str_clear(&counter_string);
    }
}

/*          IF STATEMENT            */
void generate_if_label(string_t *insert_to, char *label_or_jump)
{
    str_insert(insert_to, label_or_jump);
    str_add_char(insert_to, '_');
    str_insert(insert_to, local_tab->key.str);
    str_add_char(insert_to, '_');
    str_insert_int(insert_to, local_tab->depth);
    str_add_char(insert_to, '_');
    // create counter based on previous if counter
    str_insert_int(insert_to, local_tab->next->if_cnt);
    str_add_char(insert_to, '_');
    // determine, whether this part is before or after else
    str_insert_int(insert_to, local_tab->next->after_else);
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

void generate_if_else()
{
    string_t label_name;
    str_init(&label_name);

    generate_if_label(&label_name, "jumpifneq ");
    str_insert(&label_name, "_else GF@bool bool@true");
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

/*          END IF STATEMENT            */


/*          WHILE STATEMENT             */
void generate_while_label(string_t *insert_to, char *label_or_jump)
{
    str_insert(insert_to, label_or_jump);
    str_add_char(insert_to, '_');
    str_insert(insert_to, local_tab->key.str);
    str_add_char(insert_to, '_');
    str_insert_int(insert_to, local_tab->depth);
    str_add_char(insert_to, '_');
    str_insert_int(insert_to, local_tab->next->while_cnt);
}

void generate_while_start()
{
    string_t label_name;
    str_init(&label_name);

    generate_while_label(&label_name, "label ");
    str_insert(&label_name, "_start");
    strcat(INST, label_name.str);
    ADD_NEWLINE();

    str_free(&label_name);
}

void generate_while_skip()
{
    string_t label_name;
    str_init(&label_name);

    generate_while_label(&label_name, "jumpifneq ");
    str_insert(&label_name, "_skip GF@bool bool@true");
    strcat(INST, label_name.str);
    ADD_NEWLINE();

    str_free(&label_name);
}

void generate_while_end()
{
    string_t label_name;
    str_init(&label_name);

    generate_while_label(&label_name, "jump ");
    str_insert(&label_name, "_start");
    strcat(INST, label_name.str);
    ADD_NEWLINE();

    str_clear(&label_name);

    generate_while_label(&label_name, "label ");
    str_insert(&label_name, "_skip");
    strcat(INST, label_name.str);
    ADD_NEWLINE();

    str_free(&label_name);
}
/*          END WHILE STATEMENT             */


/*          EXPRESSION              */
void generate_expr_start()
{
    ADD_INST_N("#EXPR START");
}

void generate_expr_end()
{
    ADD_INST_N("#EXPR END");
}

void generate_strlen()
{
    // pop operand into GF@output
    ADD_INST_N("pops GF@arg1");
    ADD_INST_N("strlen GF@output GF@arg1");
    ADD_INST_N("pushs GF@output");
}

void generate_concat()
{
    // pop operands into GF@arg1 GF@arg2
    ADD_INST_N("pops GF@arg2");
    ADD_INST_N("pops GF@arg1");
    ADD_INST_N("concat GF@output GF@arg1 GF@arg2");
    ADD_INST_N("pushs GF@output");
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
        ADD_INST_N("pops GF@arg2");
        ADD_INST_N("pops GF@arg1");

        // compare < only
        ADD_INST_N("pushs GF@arg1");
        ADD_INST_N("pushs GF@arg2");
        ADD_INST_N("lts");

        // compare ==
        ADD_INST_N("pushs GF@arg1");
        ADD_INST_N("pushs GF@arg2");
        ADD_INST_N("eqs");

        // compare <=
        ADD_INST_N("ors");
        break;

    case GREAT:
        ADD_INST_N("gts");
        break;

    case GREAT_EQ:
        // store variables
        ADD_INST_N("pops GF@arg2");
        ADD_INST_N("pops GF@arg1");

        // compare > only
        ADD_INST_N("pushs GF@arg1");
        ADD_INST_N("pushs GF@arg2");
        ADD_INST_N("gts");

        // compare ==
        ADD_INST_N("pushs GF@arg1");
        ADD_INST_N("pushs GF@arg2");
        ADD_INST_N("eqs");

        // compare >=
        ADD_INST_N("ors");
        break;

    default:
        break;
    }

    ADD_INST_N("pops GF@bool");
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
        // division by 0 check
        ADD_INST_N("pops GF@arg1");
        ADD_INST_N("pushs GF@arg1");
        ADD_INST_N("jumpifeq _div_by_zero GF@arg1 float@0x0p+0");
        ADD_INST_N("divs");
        break;

    case DIV_INT:
        // division by 0 check
        ADD_INST_N("pops GF@arg1");
        ADD_INST_N("pushs GF@arg1");
        ADD_INST_N("jumpifeq _div_by_zero GF@arg1 int@0");
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
        // check both operands are not nil
        ADD_INST_N("pops GF@arg1");
        ADD_INST_N("pops GF@arg2");
        ADD_INST_N("pushs GF@arg2");
        ADD_INST_N("pushs GF@arg1");
        ADD_INST_N("type GF@bool GF@arg1");
        ADD_INST_N("jumpifeq _nil_with_operator GF@bool string@nil");
        ADD_INST_N("type GF@bool GF@arg2");
        ADD_INST_N("jumpifeq _nil_with_operator GF@bool string@nil");

        generate_push_arithmetic(op);
        break;

    case EQ:
    case NOT_EQ:
        generate_push_compare(op);
        break;

    case LESS:
    case LESS_EQ:
    case GREAT:
    case GREAT_EQ:
        // check both operands are not nil
        ADD_INST_N("pops GF@arg1");
        ADD_INST_N("pops GF@arg2");
        ADD_INST_N("pushs GF@arg2");
        ADD_INST_N("pushs GF@arg1");
        ADD_INST_N("type GF@bool GF@arg1");
        ADD_INST_N("jumpifeq _nil_with_operator GF@bool string@nil");
        ADD_INST_N("type GF@bool GF@arg2");
        ADD_INST_N("jumpifeq _nil_with_operator GF@bool string@nil");

        generate_push_compare(op);
        break;

    case STR_LEN:
        ADD_INST_N("pops GF@arg1");
        ADD_INST_N("pushs GF@arg1");
        ADD_INST_N("type GF@bool GF@arg1");
        ADD_INST_N("jumpifeq _nil_with_operator GF@bool string@nil");
        generate_strlen();
        break;

    case CONCAT:
        ADD_INST_N("pops GF@arg1");
        ADD_INST_N("pops GF@arg2");
        ADD_INST_N("pushs GF@arg2");
        ADD_INST_N("pushs GF@arg1");
        ADD_INST_N("type GF@bool GF@arg1");
        ADD_INST_N("jumpifeq _nil_with_operator GF@bool string@nil");
        ADD_INST_N("type GF@bool GF@arg2");
        ADD_INST_N("jumpifeq _nil_with_operator GF@bool string@nil");
        generate_concat();
        break;

    default:
        break;
    }
}

void generate_push_operand(token_t *token)
{
    ADD_INST("pushs ");

    switch (token->type)
    {
    case TOK_STRING:
        generate_string(token->attribute.s);
        break;

    case TOK_DECIMAL:
        generate_decimal(token->attribute.decimal);
        break;

    case TOK_INT:
        generate_int(token->attribute.number);
        break;

    case TOK_ID:
        strcat(INST, "LF@");
        generate_name(buffer, token->attribute.s);
        break;

    case TOK_KEYWORD:
        strcat(INST, "nil@nil");
        break;

    default:
        break;
    }

    ADD_NEWLINE();
}
/*          END EXPRESSION          */

void generate_num_conversion(unsigned index)
{
    static int counter = 0;

    string_t s;
    str_init(&s);

    str_insert_int(&s, counter);
    ADD_INST("jumpifeq _conv_nil");
    strcat(INST, s.str);
    strcat(INST, " TF@%");
    str_clear(&s);
    str_insert_int(&s, index);
    strcat(INST, s.str);
    strcat(INST, " nil@nil");
    ADD_NEWLINE();


    ADD_INST("int2float ");
    strcat(INST, "TF@%");
    strcat(INST, s.str);

    strcat(INST, " TF@%");
    strcat(INST, s.str);
    ADD_NEWLINE();

    str_clear(&s);
    str_insert_int(&s, counter);
    ADD_INST("label _conv_nil");
    strcat(INST, s.str);
    ADD_NEWLINE();

    str_free(&s);

    counter++;
}

void generate_int_to_num()
{
    ADD_INST_N("int2floats");
}