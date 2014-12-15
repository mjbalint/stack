/**
 * @file
 * Interactive stack command. 
 *
 * This command creates a simple interactive shell that allows the user
 * manipulate a single stack. The shell supports the following commands:
 *
 * <ul>
 *   <li><b>push</b> <i><string></i> -- Push a string onto the stack.
 *   <li><b>pop</b> -- Remove string from top of stack. 
 *   <li><b>peek</b> -- Look at top string without removing it.
 *   <li><b>show</b> -- Show current contents of stack.
 *   <li><b>help</b> -- Show command list.
 *   <li><b>size</b> -- Report number of items in stack.
 *   <li><b>quit</b> -- Exit shell
 * </ul>
 *
 * @par Design
 * The command makes use of the libstack.so shared library that is the
 * core of the https://github.com/mbjalint/stack repository. It uses the
 * include/stack.h interface to create an manipulate a single global stack. 
 *
 * The push command converts the rest of the line into a string and copies
 * it into the stack. In this way, we can test the variable-length data
 * portion of the include/stack.h interface.
 *
 * The shell uses a simple, single-keyword parser. It implements first
 * unique match semantics and is case-insensitive. For example, any of
 * 'q', 'qu', 'qui', 'quit', 'Q', 'QU', 'QUI' or 'QUIT' will exit the shell
 * since there are no other commands that start with 'q'. The parser will
 * not match commands with extra characters, such as 'quitX'. There are several
 * commands starting with 'p' so either 'p' or 'P' will report an insufficient
 * match. 'pu', 'po' and 'pe' will all be accepted.
 *
 * The parser uses a simple array of commands to determine the legal keywords
 * and associated callback functions. New commands can be added to the shell
 * by adding extra entries into the array.
 * 
 * @par Limitations
 * <ol>
 *    <li>Command always runs interactive mode so there is limited support
 *        for i/o redirection and command chaining.
 *    <li>Parser is serviceable for this simple program but would need
 *        extensive re-architecture to do anything fancy (deep keyword
 *        hierarchies, inline validation, typed inputs, etc). It would
 *        probably be best to make use of third-party parser library. 
 *    <li>Only one stack.
 *    <li>'show' command currently dumps contents of stack using the
 *         stack_print() debug function so it includes lots of internal
 *         details and shows the strings as a hex-dump. Should be replaced
 *         with a more friendly format.
 * </ol>
 * 
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
 *     You should have received a copy of the GNU Lesser Public License
 *     along with https://github.com/mjbalint/stack.  If not,
 *     see <http://www.gnu.org/licenses/>. 
 */

#include "../include/stack.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Stack to manipulate
 */
static stack_t *g_stack_p = NULL;

/**
 * Callback function type for parsed commands.
 *
 * @param[in]
 *     Arguments for command.
 * @retval true
 *     Continue execution
 * @retval false
 *     End program
 */
typedef bool (*stack_cmd_command_fn)(const char* args);

/**
 * A parser command.
 */
typedef struct stack_cmd_command_ {
    /**
     * Name of command.
     */
    const char* name;
    /**
     * Command argument format
     */
    const char* args;
    /**
     * Command description
     */
    const char* help;
    /**
     * Callback function to run if command is parsed. 
     */
    stack_cmd_command_fn cb;
} stack_cmd_command_t;

/*
 * Forward declaration
 */
static bool stack_cmd_help(const char* args);

/**
 * Handle 'peek' command.
 *
 * @retval true
 *     Continue processing.
 * @retval false
 *     Exit program. 
 */
static bool stack_cmd_peek (__attribute__((unused)) const char* args)
{
    stack_err_e err           = STACK_E_OK;      /* Operation return code     */
    char        out_args[128] = "";              /* Copied value buffer       */
    size_t      out_args_size = 0;               /* Copied value buffer size  */

    out_args_size = sizeof(out_args);
    err = stack_peek(g_stack_p, out_args, &out_args_size);
    if (stack_err_e_is_error(err)) {
        printf("Error: Can't peek: %d(%s)\n",
               err, stack_err_e_to_string(err));
    } else {
        out_args[out_args_size] = '\0';
        printf("'%s' is at top of stack\n", out_args);
    }

    return (true);
}

/**
 * Handle 'pop' command.
 *
 * @retval true
 *     Continue processing.
 * @retval false
 *     Exit program. 
 */
static bool stack_cmd_pop (__attribute__((unused)) const char* args)
{
    stack_err_e err           = STACK_E_OK;      /* Operation return code     */
    char        out_args[128] = "";              /* Popped value buffer       */
    size_t      out_args_size = 0;               /* Popped value buffer size  */

    out_args_size = sizeof(out_args);
    err = stack_pop(g_stack_p, out_args, &out_args_size);
    if (stack_err_e_is_error(err)) {
        printf("Error: Can't pop: %d(%s)\n",
               err, stack_err_e_to_string(err));
    } else {
        out_args[out_args_size] = '\0';
        printf("Popped '%s' off the stack\n", out_args);
    }

    return (true);
}

/**
 * Handle 'push' command.
 *
 * @retval true
 *     Continue processing.
 * @retval false
 *     Exit program. 
 */
static bool stack_cmd_push (const char* args)
{
    stack_err_e err = STACK_E_OK;                /* Operation return code     */

    err = stack_push(g_stack_p, args, strlen(args));
    if (stack_err_e_is_error(err)) {
        printf("Error: Can't push '%s': %d(%s)",
               args, err, stack_err_e_to_string(err));
    } else {
        printf("Pushed '%s' unto the stack\n", args);
    }

    return (true);
}

/**
 * Handle 'quit' command.
 *
 * @retval true
 *     Continue processing.
 * @retval false
 *     Exit program. 
 */
static bool stack_cmd_quit (__attribute__((unused)) const char* args)
{
    return (false);
}

/**
 * Handle 'show' command.
 *
 * @retval true
 *     Continue procesing.
 * @retval false
 *     Exit program. 
 */
static bool stack_cmd_show (__attribute__((unused)) const char* args)
{
    stack_print(g_stack_p);
    return (true);
}

/**
 * Handle 'size' command.
 *
 * @retval true
 *     Continue processing.
 * @retval false
 *     Exit program. 
 */
static bool stack_cmd_size (__attribute__((unused)) const char* args)
{
    printf("There are %lu entries in the stack.\n",
           stack_get_num_entries(g_stack_p));

    return (true);
}

/**
 * Supported commands for interactive parser.
 */
static stack_cmd_command_t g_stack_cmd_commands[] =
{
    { "help", NULL,    "Show this message",          stack_cmd_help },
    { "peek", NULL,    "Look at top entry of stack", stack_cmd_peek },
    { "pop",  NULL,    "Remove top entry of stack",  stack_cmd_pop },
    { "push", "<val>", "Add <val> to stack",         stack_cmd_push },
    { "quit", NULL,    "End program",                stack_cmd_quit },
    { "show", NULL,    "Display stack",              stack_cmd_show },
    { "size", NULL,    "Display stack size",         stack_cmd_size },
};

/**
 * Number of commands supported by parser.
 */
#define STACK_CMD_NUM_COMMANDS \
            (sizeof(g_stack_cmd_commands) / sizeof(stack_cmd_command_t))

/**
 * Command 'name' field header.
 */
#define STACK_CMD_HDR_NAME "Command"

/**
 * Command 'help' field header.
 */
#define STACK_CMD_HDR_HELP "Description"


/**
 * Print help message to STDOUT.
 */
static bool stack_cmd_help (__attribute__((unused)) const char* args)
{
    unsigned int         i,j          = 0;       /* Loop index counter        */
    size_t               max_name_len = 0;       /* Maximum command name size */
    size_t               max_help_len = 0;       /* Maximum help string size  */
    stack_cmd_command_t *command_p    = NULL;    /* Current command           */
    size_t               cmd_name_len = 0;       /* Current command name size */
    size_t               cmd_help_len = 0;       /* Current help string size  */

    /*
     * Determine column sizes.
     */
    max_name_len = strlen(STACK_CMD_HDR_NAME);
    max_help_len = strlen(STACK_CMD_HDR_HELP);
    for (i = 0; i < STACK_CMD_NUM_COMMANDS; i++) {
        command_p = &(g_stack_cmd_commands[i]);

        cmd_name_len = strlen(command_p->name);
        if (NULL != command_p->args) {
            cmd_name_len += 1 + strlen(command_p->args);
        }
        if (cmd_name_len > max_name_len) {
            max_name_len = cmd_name_len;
        }

        cmd_help_len = strlen(command_p->help);
        if (cmd_help_len > max_help_len) {
            max_help_len = cmd_help_len;
        }
    }

    /*
     * Print header, which will be of the form:
     *
     * Command      Description
     * ===========  ==============
     */
    printf(STACK_CMD_HDR_NAME "  ");
    cmd_name_len = strlen(STACK_CMD_HDR_NAME);
    for (i = 0; i < (max_name_len - cmd_name_len); i++) {
        putchar(' ');
    }
    printf(STACK_CMD_HDR_HELP "\n");
    for (i = 0; i < max_name_len; i++) {
        putchar('=');
    }
    printf("  ");
    for (i = 0; i < max_help_len; i++) {
        putchar('=');
    }
    printf("\n");

    /*
     * Print commands.
     */
    for (i = 0; i < STACK_CMD_NUM_COMMANDS; i++) {
        command_p = &(g_stack_cmd_commands[i]);

        printf("%s", command_p->name);
        cmd_name_len = strlen(command_p->name);
        if (NULL != command_p->args) {
            printf(" %s", command_p->args);
            cmd_name_len += 1 + strlen(command_p->args);
        }
        for (j = 0; j < (max_name_len - cmd_name_len); j++) {
            putchar(' ');
        }

        printf("  %s\n", command_p->help);
    }

    return (true);
}

/**
 * Parse next line.
 *
 * @retval true
 *     Continue execution
 * @retval false
 *     End program.
 */
static bool stack_cmd_parse_line (void)
{
    bool match[STACK_CMD_NUM_COMMANDS];          /* Command match status      */
    int  c        = EOF;                         /* Current character         */
    char cmd[128] = "";                          /* Command string so far     */
    unsigned int  cmd_pos  = 0;                          /* Position in command string*/
    int  num_matches = 0;                        /* # Matching commands so far*/
    stack_cmd_command_t *command_p    = NULL;    /* Current command           */
    char args[128] = "";                         /* Argument string           */
    unsigned int args_pos = 0;                          /* Position in argument str  */
    bool is_continue = true;                     /* Continue to next line?    */
    unsigned int i = 0;                          /* Loop index counter        */

    /*
     * Strip initial whitespace
     */
    for (;;) {
        c = getchar();
        if (EOF == c) {
           /*
            * No more input.
            */
           return (false);
        } else if ('\n' == c) {
           /*
            * Ignore blank lines.
            */
           return (true);
        } else if (('\t' != c) && (' ' != c)) {
           /*
            * We have reached the end of the whitespace. Start main processing.
            */
           break;
        }
    }

    /*
     * Start by treating all possible commands as matches.
     * We will eliminate possiblities with each input character.
     */
    memset(match, true, sizeof(match));
    num_matches = STACK_CMD_NUM_COMMANDS;
    cmd_pos = 0;    
    for (;;) {
        /*
         * Truncate command if we run out of buffer space to store it,
         * otherwise add next character to command and see update the
         * match list.
         */
        if (cmd_pos < sizeof(cmd)) {
            cmd[cmd_pos] = c;

            if (num_matches > 0) {
                for (i = 0; i < STACK_CMD_NUM_COMMANDS; i++) {
                    if (match[i]) {
                        command_p = &(g_stack_cmd_commands[i]);
                        if ((cmd_pos >= (strlen(command_p->name))) ||
                            (toupper(command_p->name[cmd_pos]) != toupper(c))) {
                            match[i] = false;
                            num_matches--;
                        }
                    }
                }
            }
        }

        /*
         * Continue until we reach end of command word.
         */
        cmd_pos++;
        c = getchar();
        if ((EOF == c) || ('\n' == c) || (' ' == c) || ('\t' == c)) { 
            cmd[cmd_pos] = '\0';
            break;
        }
    }

    /*
     * Populate argument list from remaining characters on line, after
     * first stripping out any whitespace between the command and
     * first argument.
     */
    while ((' ' == c) || ('\t' == c)) {
        c = getchar();
    }
    args_pos = 0;
    while ((c != '\n') && (c != EOF)) {
        /*
         * Truncate argument if we run out of buffer space to store it.
         * Otherwise, add the next character to the argument.
         */
        if (args_pos < sizeof(args)) {
            args[args_pos] = c;
            args_pos++;
        }

        c = getchar();
    }
    args[args_pos] = '\0';

    if (0 == num_matches) {
        printf("Unknown command '%s'. Type HELP for command list.\n", cmd);
    } else if (1 == num_matches) {
        for (i = 0; i < STACK_CMD_NUM_COMMANDS; i++) {
            if (match[i]) {
                command_p = &(g_stack_cmd_commands[i]);
                is_continue = command_p->cb(args);
                break;
            }
        }
    } else {
        /*
         * Multiple matches. 
         */
        printf("Incomplete command '%s'. Type HELP for command list.\n", cmd);
    }

    if ((!is_continue) || (EOF == c)) {
        return (false);
    }
    return (true);
}

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
    /*
     * Allocate a new stack.
     */
    g_stack_p = stack_alloc();
    if (NULL == g_stack_p) {
        printf("Sorry, I can't create a stack for you.");
        return (-1);
    }

    /*
     * Give welcome message then continue processing lines until done.
     */
    (void)stack_cmd_help(NULL);
    for (;;) {
        printf("> ");
        
	if (! stack_cmd_parse_line()) {
           break;
        }
    }

    /*
     * Free stack
     */
    stack_free(g_stack_p);

    return (0);
}
