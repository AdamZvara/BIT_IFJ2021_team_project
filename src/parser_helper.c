/**
 * VUT IFJ Project 2021.
 *
 * @file p_helper_helper.c
 *
 * @brief Helper structure when dealing with p_helpertion declaration, definition, p_helpertion call etc...
 *
 * @author Vojtěch Eichler
 * @author Václav Korvas
 * @author Tomáš Matuš
 * @author Adam Zvara
 */

#include "parser_helper.h"
#include "error.h"      // ERROR TYPES

parser_helper_t *p_helper_create()
{
    parser_helper_t *f = malloc(sizeof(*f));
    if (f == NULL) {
        return NULL;
    }

    f->id_first = NULL;
    f->id_last = NULL;
    f->func = NULL;
    f->func_found = false;
    f->par_counter = 0;
    f->assign = false;
    if (str_init(&f->temp)) {
        return NULL;
    }
    if (str_init(&f->status)) {
        return NULL;
    }

    return f;
}

void p_helper_clear(parser_helper_t *f)
{
    // TODO: free identifier linked list
    f->id_first = NULL;
    f->id_last = NULL;
    f->assign = false;
    f->func = NULL;
    f->func_found = false;
    f->par_counter = 0;
    str_clear(&f->temp);
}

void p_helper_clear_string(parser_helper_t *f)
{
    str_clear(&f->temp);
}

void p_helper_dispose(parser_helper_t *f)
{
    str_free(&f->temp);
}

int p_helper_add_identifier(parser_helper_t *f, struct local_data *id)
{
    struct identifiers *new_id = malloc(sizeof(*new_id));
    if (new_id == NULL) {
        return ERROR_INTERNAL;
    }

    new_id->next = NULL;
    new_id->data = id;

    // insert first id into empty list
    if (f->id_first == NULL) {
        f->id_first = f->id_last = new_id;
    } else {
        f->id_last->next = new_id;
        f->id_last = new_id;
    }

    return SUCCESS;
}

void p_helper_delete_identifier(parser_helper_t *f)
{
    struct identifiers *del = f->id_first;
    if (del == NULL) {
        return;
    }

    f->id_first = del->next;
    free(del);

    if (f->id_first == NULL) {
        f->id_last = NULL;
    }

}

int p_helper_set_params(parser_helper_t *f, keyword_t kw)
{
    switch (kw) {
    case KW_STRING:
        if (f->func_found) {
                str_add_char(&f->temp, 's');
            } else {
                str_add_char(&f->func->params, 's');
            }
        break;

    case KW_INTEGER:
        if (f->func_found) {
                str_add_char(&f->temp, 'i');
            } else {
                str_add_char(&f->func->params, 'i');
            }
        break;

    case KW_NUMBER:
        if (f->func_found) {
                str_add_char(&f->temp, 'n');
            } else {
                str_add_char(&f->func->params, 'n');
            }
        break;

    default:
        break;
    }

    return 0;
}

int p_helper_set_retvals(parser_helper_t *f, keyword_t kw)
{
    switch (kw) {
    case KW_STRING:
        if (f->func_found) {
            str_add_char(&f->temp, 's');
        } else {
            str_add_char(&f->func->retvals, 's');
        }
        break;

    case KW_INTEGER:
        if (f->func_found) {
            str_add_char(&f->temp, 'i');
        } else {
            str_add_char(&f->func->retvals, 'i');
        }
        break;

    case KW_NUMBER:
        if (f->func_found) {
            str_add_char(&f->temp, 'n');
        } else {
            str_add_char(&f->func->retvals, 'n');
        }
        break;

    default:
        break;
    }

    return 0;
}

int p_helper_call_params_const(parser_helper_t *f, token_type_t type)
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

int p_helper_call_params_id(parser_helper_t *f, string_t name)
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