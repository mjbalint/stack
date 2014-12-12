/**
 * @file
 * Stack -- Implementation 
 *
 * A stack is a basic collection with last-in, first-out LIFO behavior.
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
 *     You should have received a copy of the GNU General Public License
 *     along with https://github.com/mjbalint/stack.  If not,
 *     see <http://www.gnu.org/licenses/>. 
 */

#include "../include/stack.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/**
 * Stack operation return code to debug string array, indexed on return code
 * values.
 */
static char* stack_err_e_strings[STACK_NUM_ERR] =
{
    [STACK_E_OK] = "OK",
    [STACK_E_FULL] = "FULL",
    [STACK_E_INVALID] = "INVALID",
    [STACK_E_NOMEM] = "NOMEM",
    [STACK_E_EMPTY] = "EMPTY",
    [STACK_E_INTERNAL] = "INTERNAL",
    [STACK_E_BUF_OVERFLOW] = "BUFOVERFLOW",
    [STACK_E_MAX_REFCOUNT] = "MAXREFCOUNT",
};

/*
 * Get debugging string for given stack operation return code.
 *
 * See "../include/stack.h" for API details.
 */
const char* stack_err_e_to_string (stack_err_e err)
{
    /*
     * We will use the enumeration value as an index into the conversion array,
     * so first make sure that it is within bounds. 
     */
    if (stack_err_e_is_valid(err)) {
        return (STACK_ERR_UNKNOWN_STR);
    }

    /*
     * All enumerations should have a corresponding entry in the conversion
     * array, but we should guard against the possiblity that a new enumeration
     * is added without a proper update to the array. In such a case, the
     * array entry will be initialized to the default value of 0 and we can
     * thus use a NULL check to cover it. 
     */
    if (NULL == stack_err_e_strings[err]) {
        return (STACK_ERR_UNKNOWN_STR);
    }

    return (stack_err_e_strings[err]);
}

/**
 * Magic number value for a properly initialized stack.
 *
 * @note
 *     It is supposed to look a little like 'STACK'.
 */
#define STACK_MAGIC 0x57AC1

/**
 * A stack
 */
struct stack_ {
    /**
     * Magic number to identify a properly intialized stack.
     */
    unsigned long long magic;
    /**
     * Stack element buffer.
     */
    char buf[1024];
    /**
     * Start of top entry in stack. Each entry consists of the size of the
     * the user data followed by the user data itself. 
     */
    size_t *top_entry_size_p;
    /**
     * Number of entries in the stack.
     */
    size_t num_entries;
    /**
     * Reference count. 
     */
    unsigned int refcount;
};

/*
 * Determine whether or not given stack is valid.
 *
 * See ../include/stack.h for API details.
 */
bool stack_is_valid (stack_t *stack_p)
{
    return ((NULL != stack_p) && (STACK_MAGIC == stack_p->magic)); 
}

/*
 * Allocate a new stack.
 *
 * See ../include/stack.h for API details. 
 */
stack_t* stack_alloc_custom (size_t max_entries,
                             size_t max_entry_size,
                             size_t default_entry_size,
                             size_t max_size)
{
    stack_t *new_stack_p = NULL;                 /* Newly allocated stack     */

    new_stack_p = malloc(sizeof(stack_t));
    if (NULL == new_stack_p) {
        return (NULL);
    }

    /*
     * Initialize the stack. Since there are no entries, the values in the
     * data fields don't matter and we don't need to set them.
     */
    new_stack_p->num_entries = 0;
    new_stack_p->refcount = 1;

    return (new_stack_p);
}

/*
 * Get number of entries in a stack.
 *
 * See ../include/stack.h for API details.
 */
size_t stack_get_num_entries (stack_t *stack_p)
{
    if (! stack_is_valid(stack_p)) {
        return (0);
    }

    return (stack_p->num_entries);
}

/*
 * Push copy of given entry onto a stack. 
 *
 * See ../include/stack.h for API details.
 */
stack_err_e stack_push (stack_t *stack_p, void *entry_p, size_t entry_size)
{
    size_t *buf_entry_size_p = NULL;                 /* Entry size in buffer  */
    void   *buf_entry_p      = NULL;                 /* Entry in buffer       */

    /*
     * Check inputs.
     */
    if (! stack_is_valid(stack_p)) {
        return (STACK_E_INVALID);
    }
    if (NULL == entry_p) {
        return (STACK_E_INVALID);
    }
    if (entry_size < 1) {
        return (STACK_E_INVALID);
    }

    /*
     * Find the start of the new entry in the buffer.
     */
    if (0 == stack_p->num_entries) {
        /*
         * This is the first entry so start from the beginning of the buffer.
         */
        buf_entry_size_p = (size_t *)buf;
    } else {
        /*
         * Place this entry after the current top entry.
         *
         * To get to the beginning of the new top entry, we start at the 
         * beginning of the current top entry and skip over the size and data
         * fields. The size field is a fixed size and the data field bytes
         * come from the size field's value. 
         */
        buf_entry_size_p = stack_p->top_entry_size_p +
                           sizeof(size_t) + 
                           *(stack_p->top_entry_size_p);
    }

    /*
     * Make sure that we have enough space.
     */
    (((buf_entry_size_p - buf) + sizeof(size_t) + entry_size) > sizeof(buf)) {
        return (STACK_E_NOMEM);
    }

    /*
     * Copy data and update stack fields
     */
    *buf_entry_size_p = entry_size;
    buf_entry_p = buf_entry_size_p + sizeof(size_t);
    memcpy(buf_entry_p, entry_p, entry_size);
    stack_p->top_entry_size_p = buf_entry_size_p;
    stack_p->num_entries++;

    return (STACK_E_OK);
}

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
 *     entry_p. The inital value is ignored if entry_p is NULL.
 * @retval STACK_E_OK
 *     Successfully removed top entry from stack.
 * @retval STACK_E_BUFOVERFLOW
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
 * @retval STACK_E_BUFOVERFLOW
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
 * @retval STACK_E_MAXREF
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
static inline stack_free_and_clear(stack_t **stack_pp)
{
    if (NULL != stack_pp) {
        stack_free(*stack_pp);
        *stack_pp = NULL;
    }
}

#endif /* __STACK_H__ */
