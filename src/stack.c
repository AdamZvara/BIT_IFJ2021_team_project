/**
 * VUT IFJ Project 2021.
 *
 * @file stack.c
 *
 * @brief Implementation of stack for bottom up parsing
 *
 * @author Vojtěch Eichler
 * @author Václav Korvas
 * @author Tomáš Matuš
 * @author Adam Zvara
 */

#include <stdio.h>
#include <stdlib.h>
#include "expression.h"
#include "stack.h"

void stack_init(stack_t *stack)
{
    stack->top = NULL;
}

int stack_is_empty(stack_t *stack)
{
    return stack->top == NULL;
}

int stack_push(stack_t *stack, int value)
{
    stack_item_t *new = malloc(sizeof(stack_item_t));
    if (new == NULL) {
        return 1;
    }

    new->data = value;
    new->next = stack->top;
    stack->top = new;

    return 0;
}

int stack_push_above_term(stack_t *stack, int value)
{
    stack_item_t *curr = stack->top;
    stack_item_t *prev = NULL;

    stack_item_t *new = malloc(sizeof(stack_item_t));
    if (new == NULL) {
        return 1;
    }

    while (curr != NULL) {
        if (curr->data < HANDLE) {
            // found term
            new->data = value;

            if (prev == NULL) {
                new->next = curr;
                stack->top = new;
            } else {
                new->next = curr;
                prev->next = new;
            }
        } else {
            // NON_TERM on top
            prev = curr;
            curr = curr->next;
        }
    }

    return 0;
}

int stack_pop(stack_t *stack)
{
    if (stack->top == NULL) {
        // popping an empty stack
        return 1;
    }

    stack_item_t *pop = stack->top;
    stack->top = pop->next;
    free(pop);

    return 0;
}

stack_item_t *stack_top(stack_t *stack)
{
    return stack->top;
}

stack_item_t *stack_top_term(stack_t *stack)
{
    stack_item_t *top;
    top = stack_top(stack);

    if (top->data == NON_TERM) {
        top = top->next;
    }

    return stack->top;
}

void stack_dispose(stack_t *stack)
{
    while (!stack_pop(stack)); // empty body
}
