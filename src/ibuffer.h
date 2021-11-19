/**
 * VUT IFJ Project 2021.
 *
 * @file ibuffer.h
 * 
 * @brief Header file for instruction buffer
 * 
 * @author Vojtěch Eichler 
 * @author Václav Korvas
 * @author Tomáš Matuš
 * @author Adam Zvara
 */

#ifndef _IBUFFER_H
#define _IBUFFER_H

#include <stddef.h>

// macro for appending instruction into ibuffer
// ADD_INST(TEST) appends string "TEST" into buffer
#define ADD_INST(STR)                               \
do {                                                \
    strcpy(buffer->inst[buffer->length], STR);      \
} while (0)                                         \

// macro for appending newline to the end of current instruction
#define ADD_NEWLINE()                               \
do {                                                \
    strcat(buffer->inst[buffer->length], "\n");     \
    buffer->length++;                               \
} while (0)                                         \

#define INST buffer->inst[buffer->length]

/**
 * @struct ibuffer
 * 
 * @brief Flexible array member for storing instructions during code generation
 */
typedef struct ibuffer {
    size_t inst_size;   // Size of single line
    size_t size;        // Allocated lines
    size_t length;      // Used lines (current line)
    char *inst[];       // Flexible array member containing instructions
} ibuffer_t;

/**
 * @brief Create ibuffer and initialize it's values
 * 
 * @param buffer_size   Size of new buffer
 * @param inst_size     Size of each instruction
 * 
 * @return Pointer to allocated buffer, otherwise NULL
 */
ibuffer_t *ibuffer_create(size_t buffer_size, size_t inst_size);

/**
 * @brief Clear all intructions from buffer and set it to initialized state
 * 
 * @param buffer Pointer to instruction buffer
 */
void ibuffer_clear(ibuffer_t *buffer);

/**
 * @brief Print out instructions stored in buffer
 * 
 * @param buffer Pointer to instruction buffer
 */
void ibuffer_print(ibuffer_t *buffer);

/**
 * @brief Free allocated memory of ibuffer
 * 
 * @param buffer Pointer to instruction buffer
 */
void ibuffer_destroy(ibuffer_t *buffer);

#endif  // _IBUFFER_H