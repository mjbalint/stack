/**
 * @file
 * Test driver for stack library.
 *
 * @author Matthew Balint, mbalint@gmail.com
 * @date November 2014
 *
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

#include "../include/stack.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * Command line interface.
 *
 * @param argc
 *     Number of arguments. Currently ignored.
 * @param argv
 *     Argument list. Currently ignored.
 * @retval 0
 *     Successful completion.
 * @retval -1
 *     An error occurred.
 */
int main (__attribute__((unused)) int argc,
          __attribute__((unused)) char* argv[])
{
    stack_t      *stack_p      = NULL;            /* Stack to manipulate      */
    stack_err_e   err          = STACK_E_OK;      /* Operation return code    */
    int           i            = 0;               /* Loop index counter       */
    int           val          = 0;               /* Popped value             */
    size_t        val_size     = 0;               /* Size of popped value     */
    size_t        num_entries  = 0;               /* # of entries in stack    */

    /*
     * Allocate a new stack.
     */
    stack_p = stack_alloc();
    if (NULL == stack_p) {
        printf("Error: Can't init stack\n");
        return (-1);
    }

    if (! stack_is_empty(stack_p)) {
        printf("Error: Newly created stack not empty.\n");
        return (-1);
    }

    /*
     * Add some elements to stack.
     */
    for (i = 0; i < 10; i++) {
        err = stack_push(stack_p, &i, sizeof(i));
        if (stack_err_e_is_error(err)) {
            printf("Error: Push #%d: Can't push '%d' onto stack: %d(%s)\n",
                   (i+1), i, err, stack_err_e_to_string(err));
            return (-1);
        }
        num_entries = stack_get_num_entries(stack_p);
        if (num_entries != (size_t)(i+1)) {
            printf("Error: Push #%d: %lu entries after push but expected %d\n",
                   (i+1), num_entries, (i+1));
            return (-1);
        }

        printf("<<<<< After push #%d >>>>>\n", (i+1));
        stack_print(stack_p);
        printf("\n");
    }

    /*
     * Remove elements from stack and verify LIFO order.
     */
    for (i = 0; i < 10; i++) {
        val_size = sizeof(val);
        err = stack_pop(stack_p, &val, &val_size);
        if (stack_err_e_is_error(err)) {
            printf("Error: Pop #%d: Can't pop from stack: %d(%s)\n",
                   (i+1), err, stack_err_e_to_string(err));
            return (-1);
        }
        if (val_size != sizeof(val)) {
            printf("Error: Pop #%d: Value of size %lu but expected %lu\n",
                   (i+1), val_size, sizeof(val));
            return (-1);
        }
        if (val != (9 - i)) {
            printf("Error: Pop #%d: Value '%d' but expected %d\n", 
                   (i+1), val, (9 - i));
            return (-1);
        }
        num_entries = stack_get_num_entries(stack_p);
        if (num_entries != (size_t)(9 - i)) {
            printf("Error: Pop %d: %lu entries after pop but expected %d\n",
                   (i+1), num_entries, (9 - i));
            return (-1);
        }

        printf("<<<<< After pop #%d >>>>>\n", (i+1));
        stack_print(stack_p);
        printf("\n");
    }

    /*
     * Free stack
     */
    stack_free_and_clear(&stack_p);
    if (NULL != stack_p) {
        printf("Error: Popped value '%d' but expected %d\n", val, i);
        return (-1);
    }

    return (0);
}
