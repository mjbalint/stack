/**
 * @file
 * Stack -- Public Interface
 *
 * A stack is a basic collection with last-in, first-out LIFO behavior.
 *
 * This stack API represents a stack that stores a copy of the data for each
 * of its entries. It supports entries of different sizes. This makes it
 * suitable for use cases such as a process stack in which the goal is to
 * place mixed data together.   
 *
 * @author     Matthew Balint, mjbalint@gmail.com
 * @date       November 2014
 * @copyright
 *     Copyright (c) 2014 by Matthew Balint.
 *
 *     This file is part of https://github.com/mjbalint/stack
 *
 *     https://github.com/mjbalint/stack is free software: you can
 *     redistribute it and/or modify it under the terms of the
 *     GNU Lesser Public License as published by the
 *     Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 *
 *     https://github.com/mjbalint/stack is distributed in the hope that it
 *     will be useful, but WITHOUT ANY WARRANTY; without even the implied
 *     warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *     See the GNU Lesser Public License for more details.
 *
 *     You should have received a copy of the GNU Lesser Public License
 *     along with https://github.com/mjbalint/stack.  If not,
 *     see <http://www.gnu.org/licenses/>. 
 */

#ifndef __STACK_H__
#define __STACK_H__

#include <stdbool.h>
#include <stdlib.h>

/**
 * Stack operation return codes.
 */
typedef enum {
    /**
     * Successful completion.
     */
    STACK_E_OK = 0, 
    /**
     * Stack contains the maximum number of allowed entries.
     */
    STACK_E_FULL,
    /**
     * Invalid input parameter.
     */
    STACK_E_INVALID,
    /**
     * Out of memory 
     */
    STACK_E_NOMEM,
    /**
     * Stack has no entries.
     */
    STACK_E_EMPTY,
    /**
     * An internal error occurred. 
     */
    STACK_E_INTERNAL,
    /**
     * Buffer is too small to hold entry. 
     */
    STACK_E_BUF_OVERFLOW,
    /**
     * Cannot increase reference count of stack any further. 
     */
    STACK_E_MAX_REFCOUNT,
    /**
     * Number of error codes.
     *
     * @note
     *     This must always be the last enumeration value.
     */
    STACK_NUM_ERR
} stack_err_e;

/**
 * Determine whether or not given value is a valid stack operation return
 * code.
 *
 * @param[in] err
 *     Stack operation return code to check.
 * @retval true
 *     Stack operation return code is valid.
 * @retval false
 *     Stack operation return code is invalid.
 */
static inline bool stack_err_e_is_valid (stack_err_e err)
{
    return ((err >= 0) && (err < STACK_NUM_ERR));
}

/**
 * Does stack operation return code represent an error?
 *
 * @param[in] err
 *     Code to check. Invalid values are treated as representing error
 *     conditions.
 * @retval true
 *     Return code represents an error.
 * @retval false
 *     Return code indicates success.
 */
static inline bool stack_err_e_is_error(stack_err_e err)
{
    return (err != STACK_E_OK);
}

/**
 *  Debugging string for unknown stack operation return codes.
 */
#define STACK_ERR_UNKNOWN_STR "???"

/**
 * Get debugging string for given stack operation return code.
 *
 * @param[in] err
 *     Code to query.
 * @returns
 *     String corresponding to given return code.
 *     Unknown return codes will return STACK_ERR_UNKNOWN_STR. 
 */
extern const char* stack_err_e_to_string(stack_err_e err);

/**
 * A stack handle.
 */
typedef struct stack_ stack_t;

/**
 * Determine whether or not given stack is valid.
 *
 * Invalid stacks are rejected or ignored by most stack functions. 
 * See the descriptions of each stack operation for details on how they
 * handle an invalid stack. 
 *
 * @note
 *     This check will identify obviously incorrect stacks, such as NULL
 * stacks, but will not be able to identify all cases of corruption.
 */
extern bool stack_is_valid(const stack_t *stack_p);

/**
 * Special value indicating that the stack enforces no maximum on the
 * number of entries that it may contain.
 */
#define STACK_MAX_ENTRIES_NONE 0

/**
 * Special value indicating that the stack enforces no maximum on the
 * size of entries that it contains.
 */
#define STACK_MAX_ENTRY_SIZE_NONE 0

/**
 * Special value indicating that the stack enforces no maximum on the
 * amount of memory that it uses. 
 */
#define STACK_MAX_SIZE_NONE 0

/**
 * Standard default entry size in bytes. 
 *
 * @note
 *     Set to the size of a pointer.
 */
#define STACK_DEFAULT_ENTRY_SIZE (sizeof(void *))

/**
 * Allocate a new stack.
 *
 * @param[in] max_entries
 *     Maximum number of entries in the stack. Pass
 *     #STACK_MAX_ENTRIES_NONE to create a stack with no such limit.
 * @param[in] max_entry_size
 *     Maximum size of an entry in the stack, in bytes. Pass
 *     #STACK_MAX_ENTRY_SIZE_NONE to create a stack with no such limit.
 * @param[in] default_entry_size
 *     Default size of an entry in the stack. Pass
 *     #STACK_DEFAULT_ENTRY_SIZE to create a stack with no such limit. 
 * @param[in] max_size
 *     Maximum size of the stack in bytes. Pass #STACK_MAX_SIZE_NONE to
 *     create a stack with no such limit.
 * @returns
 *     Newly allocated stack on success, NULL on failure. Caller is
 *     responsible for freeing newly allocating object using stack_free().
 * @see
 *     stack_alloc(), stack_free()
 * @post
 *     Newly created stacks have a reference count of 1.
 */
extern stack_t* stack_alloc_custom(size_t max_entries,
                                   size_t max_entry_size,
                                   size_t default_entry_size,
                                   size_t max_size);

/**
 * Allocate a new stack using default parameters.
 *
 * @returns
 *     Newly allocated stack on success, NULL on failure. Caller is
 *     responsible for freeing newly allocating object using stack_free().
 * @see
 *     stack_alloc_custom(), stack_free()
 * @post
 *     Newly created stacks have a reference count of 1.
 */
static inline stack_t* stack_alloc (void)
{
    return (stack_alloc_custom(STACK_MAX_ENTRIES_NONE,
                               STACK_MAX_ENTRY_SIZE_NONE,
                               STACK_DEFAULT_ENTRY_SIZE,
                               STACK_MAX_SIZE_NONE));
}

/**
 * Get number of entries in a stack.
 *
 * @param[in] stack_p
 *     Stack to query. Invalid stacks are treaty as empty.
 * @returns
 *     Number of entries in the stack.
 */
extern size_t stack_get_num_entries(stack_t *stack_p);

/**
 * Determine whether or not stack has any entries.
 *
 * @param[in] stack_p
 *     Stack to query. Invalid stacks are treated as empty.
 */
static inline bool stack_is_empty(stack_t *stack_p)
{
    return (0 == stack_get_num_entries(stack_p));
}

/**
 * Push copy of given entry onto a stack.
 *
 * @param[in] stack_p
 *     Stack to update.
 * @param[in] entry_p
 *     Entry to copy to top of stack.
 * @param[in] entry_size
 *     Size of entry in bytes. 
 * @retval STACK_E_OK
 *     Successfully added entry.
 * @retval STACK_E_FULL
 *     Stack already holds maximum number of entries.
 * @retval STACK_E_INVALID
 *     Invalid parameter.
 * @retval STACK_E_NOMEM
 *     Out of memory.
 * @retval STACK_E_INTERNAL
 *     Another error occurred.
 * @see
 *     stack_peek(), stack_pop()
 */
extern stack_err_e stack_push(stack_t *stack_p,
                              const void *entry_p,
                              size_t entry_size);

/**
 * Remove the top entry from a stack and return a copy of it.
 *
 * @param[in] stack_p
 *     Stack to update
 * @param[in] entry_p
 *     On success, if not NULL, entry will be copied to this buffer.
 * @param[in,out] entry_size_p
 *     Initially, must be set to size, in bytes, of entry_p buffer.
 *     On success, will be updated with number of bytes copied to
 *     entry_p. The initial value is ignored if entry_p is NULL.
 * @retval STACK_E_OK
 *     Successfully removed top entry from stack.
 * @retval STACK_E_BUF_OVERFLOW
 *     entry_p buffer is too small to hold full value of top entry.
 * @retval STACK_E_EMPTY
 *     No entries in stack.
 * @retval STACK_E_INVALID
 *     Invalid parameter.
 * @retval STACK_E_INTERNAL
 *     Another error occurred.
 * @see
 *     stack_peek(), stack_push()
 */
extern stack_err_e stack_pop(stack_t *stack_p,
                             void *entry_p,
                             size_t *entry_size_p);

/**
 * Look at top entry of stack.
 *
 * @param[in] stack_p
 *     Stack to query
 * @param[out] entry_p
 *     On success, if not NULL, entry will be copied to this buffer.
 * @param[out] entry_size_p
 *     On success, if not NULL, will be updated with the size, in bytes,
 *     of the top entry of the stack. 
 * @retval STACK_E_OK
 *     Successfully queried top entry of stack.
 * @retval STACK_E_BUF_OVERFLOW
 *     entry_p buffer is too small to hold full value of top entry.
 * @retval STACK_E_EMPTY
 *     No entries in stack.
 * @retval STACK_E_INVALID
 *     Invalid parameter.
 * @retval STACK_E_INTERNAL
 *     Another error occurred.
 * @see
 *     stack_push(), stack_pop()
 */
extern stack_err_e stack_peek(const stack_t *stack_p,
                              void *entry_p,
                              size_t *entry_size_p);

/**
 * Increment reference count of stack.
 *
 * The reference count of a stack should be incremented whenever its
 * pointer is copied. This will prevent one part of the system from
 * releasing the stack's memory while it is still in use elsewhere.
 *
 * @param[in] stack_p
 *     Stack to update.
 * @retval STACK_E_OK
 *     Successfully incremented reference count. Caller is responsible
 *     for decrementing the reference count using stack_free().
 * @retval STACK_E_INVALID
 *     Invalid stack.
 * @retval STACK_E_MAX_REFCOUNT
 *     Stack already has the maximum number of references. 
 * @see
 *     stack_alloc(), stack_get_refcount(), stack_free(),
 */
extern stack_err_e stack_incr_refcount(stack_t *stack_p);

/**
 * Get number of explicit references to the stack.
 *
 * @param[in] stack_p
 *     Stack to query. Invalid stacks are considered to have 0 references.
 * @returns
 *     Number of references.
 * @see
 *     stack_alloc(), stack_incr_refcount(), stack_free()
 */
extern unsigned int stack_get_refcount(stack_t *stack_p);

/**
 * Decrement a stack's reference count and free its memory if there
 * are no more references to it.
 *
 * @param[in] stack_p
 *     Stack to free. Invalid stacks are considered to already be
 *     freed, so nothign will happen in such cases. 
 * @see
 *     stack_alloc(), stack_incr_refcount()
 */
extern void stack_free(stack_t *stack_p);

/**
 * Decrement a stack's reference count and free its memory if there
 * are no more references to it. Set pointer to NULL for safety.
 *  
 * @param[in,out] stack_pp
 *     Initially, points to stack to free. *stack_pp will be set to point
 *     to NULL at end of operation. Does nothing if stack_pp is NULL or
 *     points to an invalid stack. 
 * @see
 *     stack_free()
 */
static inline void stack_free_and_clear(stack_t **stack_pp)
{
    if (NULL != stack_pp) {
        stack_free(*stack_pp);
        *stack_pp = NULL;
    }
}

/**
 * Print contents of stack to STDOUT.
 *
 * @param[in] stack_p
 *     Stack to print. A suitable message will be displayed for invalid stacks.
 */
extern void stack_print(stack_t *stack_p);

#endif /* __STACK_H__ */
