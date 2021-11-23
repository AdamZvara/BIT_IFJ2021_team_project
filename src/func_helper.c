/**
 * VUT IFJ Project 2021.
 *
 * @file func_helper.c
 *
 * @brief Helper structure when dealing with function declaration, definition, function call etc...
 *
 * @author Vojtěch Eichler
 * @author Václav Korvas
 * @author Tomáš Matuš
 * @author Adam Zvara
 */

#include "func_helper.h"
#include "error.h"      // ERROR TYPES

int func_init(func_def_t *f)
{
    f->item = NULL;
    f->func_found = false;
    if (str_init(&f->temp)) {
        return ERROR_INTERNAL;
    }
    return SUCCESS;
}

void func_clear(func_def_t *f)
{
    str_clear(&f->temp);
}

void func_dispose(func_def_t *f)
{
    str_free(&f->temp);
}

int func_set_params(func_def_t *f, keyword_t kw)
{
    switch (kw) {

    case KW_STRING:
        if (f->func_found) {
                str_add_char(&f->temp, 's');
            } else {
                str_add_char(&f->item->params, 's');
            }
        break;

    case KW_INTEGER:
        if (f->func_found) {
                str_add_char(&f->temp, 'i');
            } else {
                str_add_char(&f->item->params, 'i');
            }
        break;

    case KW_NUMBER:
        if (f->func_found) {
                str_add_char(&f->temp, 'n');
            } else {
                str_add_char(&f->item->params, 'n');
            }
        break;

    default:
        break;
    }

    return 0;
}

int func_set_retvals(func_def_t *f, keyword_t kw)
{
    switch (kw) {
    case KW_STRING:
        if (f->func_found) {
            str_add_char(&f->temp, 's');
        } else {
            str_add_char(&f->item->retvals, 's');
        }
        break;

    case KW_INTEGER:
        if (f->func_found) {
            str_add_char(&f->temp, 'i');
        } else {
            str_add_char(&f->item->retvals, 'i');
        }
        break;

    case KW_NUMBER:
        if (f->func_found) {
            str_add_char(&f->temp, 'n');
        } else {
            str_add_char(&f->item->retvals, 'n');
        }
        break;

    default:
        break;
    }

    return 0;
}

int func_call_params_const(func_def_t *f, token_type_t type)
{
    switch (type) {
    case TOK_STRING:
        str_add_char(&f->temp, 's');
        break;

    case TOK_INT:
        str_add_char(&f->temp, 'i');
        break;

    case TOK_DECIMAL:
        str_add_char(&f->temp, 'n');
        break;

    default:
        break;
    }

    return 0;
}

int func_call_params_id(func_def_t *f, string_t name)
{
    if (local_tab == NULL) {
        return 1;
    }

    struct local_data *id = local_find(local_tab, name);
    if (id == NULL) {
        return 1;
    }

    switch (id->type) {
    case STR_T:
        str_add_char(&f->temp, 's');
        break;

    case INT_T:
        str_add_char(&f->temp, 'i');
        break;

    case NUM_T:
        str_add_char(&f->temp, 'n');
        break;

    default:
        break;
    }

    return 0;
}