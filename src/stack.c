/**
 * @file
 * Stack -- Implementation 
 *
 * A stack is a basic collection with last-in, first-out LIFO behavior.
 *
 * This implemetation stores a copy of the data for each entry in a buffer.
 * It supports entries of variable size. 
 *
 * @par Design
 * The following diagram shows the layout of the stack's buffer. Entries
 * are stored as (size, data) tuples, with a fixed-size size field followed
 * by a variable sized data field.
 *  
 *
 *                      
 *                  ,-- Top-of-stack pointer
 *                  |
 *                  V
 * [________________SSSSdddddddddSSSSddSSSSdddd]
 *  ^               ^   ^        ^   ^ ^   ^
 *  |               |   |        |   | |   |
 *  |               |   |        |   | |   |
 *  |               |   |        |   | |   `- Data for last entry in stack
 *  |               |   |        |   | `- Size of last entry in stack
 *  |               |   |        |   `- Data for next (2nd) entry in stack
 *  |               |   |        `- Size of next (2nd) entry in stack
 *  |               |   `- Data for top entry in stack. 
 *  |               `- Size of top entry in stack
 *  `- Start of free buffer space
 *
 * @par Limitations
 *    We are currently ignoring the stack configuration parameters and instead
 *    use a fixed-size buffer. We will add support for the configuration in
 *    a future update.
 *
 *    We could support 0-length entries in this implementation but they are
 *    not allowed by the current version of the stack interface.
 *
 *    We use size_t for the size fields, which is necessary for enormous
 *    entries but is overkill for stacks which contain mostly small entries.
 *    We could consider adding a type field for the size so that we can
 *    use a smaller integer type when possible. This would reduce overhead
 *    for stacks with lots of small entries at the expense of those with
 *    fewer large entries. 
 *
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
    [STACK_E_OK]           = "OK",
    [STACK_E_FULL]         = "FULL",
    [STACK_E_INVALID]      = "INVALID",
    [STACK_E_NOMEM]        = "NOMEM",
    [STACK_E_EMPTY]        = "EMPTY",
    [STACK_E_INTERNAL]     = "INTERNAL",
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
 * Maximum number of refrences to a stack.
 */
#define STACK_MAX_REFCOUNT ((unsigned int)(-1))

/**
 * A stack
 */
struct stack_ {
    /**
     * Self pointer identify a properly intialized stack.
     */
    struct stack_ *self;
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
    return ((NULL != stack_p) && (stack_p == stack_p->self)); 
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
    new_stack_p->self = new_stack_p;

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

/**
 * Determine whether or not stack is empty.
 *
 * @note
 *     This is a private implementation for use with stacks that have
 *     already been validated.
 *
 * @param[in] stack_p
 *     Stack to query. MUST BE A VALID STACK otherwise results are indeterminate.
 * @retval true
 *     Stack has no entries.
 * @retval false
 *     Stack has at least one entry.
 */
static inline bool stack_is_empty_impl (stack_t *stack_p)
{
    return (stack_p->num_entries < 1);
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
    size_t  free_buf_size    = 0;                    /* Free stack space left */
    size_t  new_entry_size   = 0;                    /* Total new entry space */

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
     * Make sure that there is enough space left in the buffer for the new entry.
     */
    free_buf_size = stack_p->top_entry_size_p - buf; 
    new_entry_size = sizeof(size_t) + entry_size;
    if (new_entry_size > free_buf_size) {
        return (STACK_E_FULL);
    }

    /*
     * Find the start of the new entry in the buffer.
     */
    if (stack_is_empty_impl(stack_p)) {
        /*
         * This is the first entry; place it at the end of the buffer.
         */
        buf_entry_size_p = (size_t *)(buf + sizeof(buf) - new_entry_size);
    } else {
        /*
         * Place this entry before the current top entry.
         *
         * To get to the beginning of the new top entry, we start at the 
         * beginning of the current top entry and skip over the size and data
         * fields. The size field is a fixed size and the data field bytes
         * come from the size field's value. 
         */
        buf_entry_size_p = stack_p->top_entry_size_p - new_entry_size; 
    }

    /*
     * Copy data for entry into buffer 
     */
    *buf_entry_size_p = entry_size;
    buf_entry_p = buf_entry_size_p + sizeof(size_t);
    memcpy(buf_entry_p, entry_p, entry_size);

    /*
     * New entry is ready so we can now update the stack content fields.
     */
    stack_p->top_entry_size_p = buf_entry_size_p;
    stack_p->num_entries++;

    return (STACK_E_OK);
}

/*
 * Remove the top entry from a stack and return a copy of it.
 *
 * See ../include/stack.h for API details.
 */
stack_err_e stack_pop (stack_t *stack_p, void *entry_p, size_t *entry_size_p)
{
    stack_err_e err = STACK_E_OK;                /* Operation return code     */

    /*
     * First copy value from top of stack. 
     */
    err = stack_peek(stack_p, entry_p, entry_size_p);
    if (stack_err_e_is_error(err)) {
        return (err);
    }

    /*
     * Remove entry from stack.
     */
    (stack_p->num_entries)--;
    if (stack_is_empty_impl(stack_p)) {
        /*
         * The top pointer is not used for empty stacks, but we set it to NULL
         * for safety.
         */
        stack_p->top_entry_size_p = NULL;
    } else {
        /*
         * Skip over fixed size field and variable data field to reach
         * beginning of next entry in the stack. 
         */
        stack_p->top_entry_size_p +=
            sizeof(size_t) + *(stack_p->top_entry_size_p);
    }

    return (STACK_E_OK);
}

/*
 * Look at top entry of stack.
 *
 * See ../include/stack.h for API details.
 */
stack_err_e stack_peek (const stack_t *stack_p,
                        void *entry_p,
                        size_t *entry_size_p)
{
    size_t  in_entry_size  = 0;                  /* Output data buffer size   */
    size_t  out_entry_size = 0;                  /* Stack entry data size     */
    void   *buf_entry_p    = NULL;               /* Entry in buffer           */

    /*
     * Check inputs.
     */
    if (! stack_is_valid(stack_p)) {
        return (STACK_E_INVALID);
    }
    if (NULL == entry_size_p) {
        return (STACK_E_INVALID);
    }
    if (NULL != entry) {
        /*
         * If an output buffer is supplied, it must have at least one byte.
         */
        in_entry_size = *entry_size_p;
        if (in_entry_size < 1) { 
            return (STACK_E_INVALID);
        }
    }

    /*
     * Nothing to do for empty stacks.
     */
    if (stack_p->num_entries < 1) {
        return (STACK_E_EMPTY);
    }

    /*
     * Copy data for entry from buffer. 
     */
    out_entry_size = *(stack_p->top_entry_size_p);
    if (out_entry_size > in_entry_size) {
        return (STACK_E_BUF_OVERFLOW);
    }
    memcpy(entry_p, buf_entry_p, out_entry_size);
    *entry_size_p = out_entry_size;

    return (STACK_E_OK);
}

/*
 * Increment reference count of stack.
 *
 * See ../include/stack.h for API details. 
 */
stack_err_e stack_incr_refcount (stack_t *stack_p)
{
    if (! stack_is_valid(stack_p)) {
        return (STACK_E_INVALID);
    }

    if (stack_p->refcount >= STACK_MAX_REFCOUNT) {
        return (STACK_E_MAX_REFCOUNT);
    }

    (stack_p->refcount)++;

    return (STACK_E_OK);
}

/*
 * Get number of explicit references to the stack.
 *
 * See ../include/stack.h for API details. 
 */
unsigned int stack_get_refcount (stack_t *stack_p)
{
    if (! stack_is_valid(stack_p)) {
        return (0);
    }

    return (stack_p->refcount);
}

/*
 * Decrement a stack's reference count and free its memory if there
 * are no more references to it.
 *
 * See ../include/stack.h for API details.
 */
void stack_free (stack_t *stack_p)
{
    if (NULL == stack_p) {
        return;
    }

    (stack_p->refcount)--;
    if (0 == stack_p->refcount) {
        free(stack_p);
    }
}
