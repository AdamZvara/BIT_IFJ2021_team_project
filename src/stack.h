/**
 * VUT IFJ Project 2021.
 *
 * @file stack.h
 *
 * @brief Header flile for stack.c
 *
 * @author Vojtěch Eichler
 * @author Václav Korvas
 * @author Tomáš Matuš
 * @author Adam Zvara
 */

#ifndef _STACK_H_
#define _STACK_H_

typedef struct stack_item {
    struct stack_item *next;
    int data;
    // more TODO?
} stack_item_t;

typedef struct {
    stack_item_t *top;
} stack_t;

/**
 * @brief Initialize stack
 *
 * @param stack New stack to initialize
 */
void stack_init(stack_t *stack);

/**
 * @brief Check whether stack is empty or not
 *
 * @param stack Initialized stack
 *
 * @return Non zero if empty else 0
 */
int stack_is_empty(stack_t *stack);

/**
 * @brief Push item on the top of a stack
 *
 * @param stack Initialized stack
 * @param value Value to push on stack
 *
 * @return 0 on success, else 1
 */
int stack_push(stack_t *stack, int value);

/**
 * @brief Push item above highest term
 *
 * @param stack Initialized stack
 * @param value Value to push on stack
 *
 * @return 0 on success, else 1
 */
int stack_push_above_term(stack_t *stack, int value);

/**
 * @brief Remove top item from a stack
 *
 * @param stack Initialized stack
 *
 * @return 0 on success, 1 when popping an empty stack
 */
int stack_pop(stack_t *stack);

/**
 * @brief Read item on the top of a stack
 *
 * @param stack Initialized stack
 *
 * @return Pointer to top item
 */
stack_item_t *stack_top(stack_t *stack);

/**
 * @brief Read term on the top of a stack
 *
 * @param stack Initialized stack
 *
 * @return Pointer to top term
 */
stack_item_t *stack_top_term(stack_t *stack);

/**
 * @brief Delete stack
 *
 * @param stack Initialized stack
 */
void stack_dispose(stack_t *stack);

/**
 * @brief Counts how many items are above HANDLE
 *
 * @param stack Initialized stack
 *
 * @return Count of items above HANDLE
 */
int items_to_handle(stack_t *stack);

int find_len_op(stack_t *stack);

/**
 * @brief Returns highest operator on stack
 *
 * @param stack Initialized stack
 *
 * @return Operator
 */
stack_item_t *get_top_operator(stack_t *stack);

#endif //_STACK_H_
