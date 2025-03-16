#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include "dshlib.h"

/*
 * Implement your exec_local_cmd_loop function by building a loop that prompts the 
 * user for input.  Use the SH_PROMPT constant from dshlib.h and then
 * use fgets to accept user input.
 * 
 *      while(1){
 *        printf("%s", SH_PROMPT);
 *        if (fgets(cmd_buff, ARG_MAX, stdin) == NULL){
 *           printf("\n");
 *           break;
 *        }
 *        //remove the trailing \n from cmd_buff
 *        cmd_buff[strcspn(cmd_buff,"\n")] = '\0';
 * 
 *        //IMPLEMENT THE REST OF THE REQUIREMENTS
 *      }
 * 
 *   Also, use the constants in the dshlib.h in this code.  
 *      SH_CMD_MAX              maximum buffer size for user input
 *      EXIT_CMD                constant that terminates the dsh program
 *      SH_PROMPT               the shell prompt
 *      OK                      the command was parsed properly
 *      WARN_NO_CMDS            the user command was empty
 *      ERR_TOO_MANY_COMMANDS   too many pipes used
 *      ERR_MEMORY              dynamic memory management failure
 * 
 *   errors returned
 *      OK                     No error
 *      ERR_MEMORY             Dynamic memory management failure
 *      WARN_NO_CMDS           No commands parsed
 *      ERR_TOO_MANY_COMMANDS  too many pipes used
 *   
 *   console messages
 *      CMD_WARN_NO_CMD        print on WARN_NO_CMDS
 *      CMD_ERR_PIPE_LIMIT     print on ERR_TOO_MANY_COMMANDS
 *      CMD_ERR_EXECUTE        print on execution failure of external command
 * 
 *  Standard Library Functions You Might Want To Consider Using (assignment 1+)
 *      malloc(), free(), strlen(), fgets(), strcspn(), printf()
 * 
 *  Standard Library Functions You Might Want To Consider Using (assignment 2+)
 *      fork(), execvp(), exit(), chdir()
 */

/* Local constant for execution error message */
#define CMD_ERR_EXECUTE "error: execution failure\n"

/* Global variable to track last external command exit code */
static int last_exit_code = 0;

/* Allocate the command buffer (_cmd_buffer) in cmd_buff_t */
int alloc_cmd_buff(cmd_buff_t *cmd_buff) {
    cmd_buff->_cmd_buffer = malloc(SH_CMD_MAX);
    if (cmd_buff->_cmd_buffer == NULL) {
        return ERR_MEMORY;
    }
    return clear_cmd_buff(cmd_buff);
}

/* Free the allocated command buffer */
int free_cmd_buff(cmd_buff_t *cmd_buff) {
    if (cmd_buff->_cmd_buffer != NULL) {
        free(cmd_buff->_cmd_buffer);
        cmd_buff->_cmd_buffer = NULL;
    }
    return OK;
}

/* Clear the command buffer structure */
int clear_cmd_buff(cmd_buff_t *cmd_buff) {
    cmd_buff->argc = 0;
    for (int i = 0; i < CMD_ARGV_MAX; i++) {
        cmd_buff->argv[i] = NULL;
    }
    return OK;
}

/*
 * build_cmd_buff:
 *   Parses the input command line (cmd_line) into tokens and stores them in
 *   the cmd_buff structure. It handles trimming of leading/trailing spaces,
 *   eliminates duplicate spaces (except within double-quoted strings), and
 *   preserves spaces inside quotes.
 *
 * Returns:
 *   OK if one or more tokens are parsed,
 *   WARN_NO_CMDS if no tokens are found.
 */
int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd_buff) {
    /* Copy the input line into the allocated buffer */
    strncpy(cmd_buff->_cmd_buffer, cmd_line, SH_CMD_MAX - 1);
    cmd_buff->_cmd_buffer[SH_CMD_MAX - 1] = '\0';

    char *p = cmd_buff->_cmd_buffer;
    
    /* Trim leading spaces */
    while (*p && isspace(*p)) {
        p++;
    }
    if (*p == '\0') {
        return WARN_NO_CMDS;
    }

    int argc = 0;
    while (*p != '\0' && argc < CMD_ARGV_MAX - 1) {
        /* Skip any extra spaces between tokens */
        while (*p && isspace(*p)) {
            p++;
        }
        if (*p == '\0')
            break;

        /* If token starts with a double quote, treat it as a quoted string */
        if (*p == '"') {
            p++;  /* Skip opening quote */
            cmd_buff->argv[argc] = p;
            /* Find the closing quote */
            while (*p && *p != '"') {
                p++;
            }
            if (*p == '"') {
                *p = '\0';  /* Terminate the token */
                p++;
            }
        } else {
            /* Non-quoted token: mark the start */
            cmd_buff->argv[argc] = p;
            /* Move until next whitespace */
            while (*p && !isspace(*p)) {
                p++;
            }
            if (*p) {
                *p = '\0'; /* Terminate the token */
                p++;
            }
        }
        argc++;
    }
    cmd_buff->argc = argc;
    return OK;
}

/*
 * match_command:
 *   Checks the first token (input) against built-in command names.
 *   Returns the corresponding Built_In_Cmds enum value.
 */
Built_In_Cmds match_command(const char *input) {
    if (strcmp(input, EXIT_CMD) == 0)
        return BI_CMD_EXIT;
    if (strcmp(input, "cd") == 0)
        return BI_CMD_CD;
    if (strcmp(input, "dragon") == 0)
        return BI_CMD_DRAGON;
    if (strcmp(input, "rc") == 0)
        return BI_RC;
    return BI_NOT_BI;
}

/*
 * exec_built_in_cmd:
 *   Executes a built-in command based on the command buffer.
 *   For:
 *     - "exit": signals to exit the command loop.
 *     - "cd": changes directory if an argument is provided.
 *     - "dragon": prints a "not implemented" message.
 *     - "rc": prints the last external command's exit code.
 *
 * Returns:
 *   BI_CMD_EXIT for exit command,
 *   BI_EXECUTED for any other built-in command.
 */
Built_In_Cmds exec_built_in_cmd(cmd_buff_t *cmd) {
    Built_In_Cmds bi = match_command(cmd->argv[0]);
    if (bi == BI_CMD_EXIT) {
        return BI_CMD_EXIT;
    } else if (bi == BI_CMD_CD) {
        /* When no argument is provided, do nothing */
        if (cmd->argc < 2) {
            /* No operation */
        } else {
            if (chdir(cmd->argv[1]) != 0) {
                perror("cd");
            }
        }
    } else if (bi == BI_CMD_DRAGON) {
        printf(M_NOT_IMPL);
    } else if (bi == BI_RC) {
        printf("%d\n", last_exit_code);
    }
    return BI_EXECUTED;
}

/*
 * exec_cmd:
 *   Implements the fork/exec pattern for executing external commands.
 *   It forks a child process that uses execvp() to run the command.
 *   The parent process waits for the child to finish and updates the global
 *   last_exit_code based on the child's exit status.
 *
 * Returns:
 *   OK on success or ERR_EXEC_CMD on fork failure.
 */
int exec_cmd(cmd_buff_t *cmd) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return ERR_EXEC_CMD;
    } else if (pid == 0) {
        /* In the child process */
        execvp(cmd->argv[0], cmd->argv);
        /* If execvp returns, an error occurred */
        perror("execvp");
        exit(errno);
    } else {
        /* In the parent process: wait for the child to complete */
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            last_exit_code = WEXITSTATUS(status);
        } else {
            last_exit_code = -1;
        }
    }
    return OK;
}

/*
 * exec_local_cmd_loop:
 *   Main command loop for the shell. It prompts the user for input, builds a
 *   command buffer, and either executes a built-in command or forks to run an
 *   external command.
 *
 * Returns:
 *   OK if the loop terminates normally, or OK_EXIT if the built-in "exit" is
 *   invoked.
 */
int exec_local_cmd_loop() {
    char line[SH_CMD_MAX];

    while (1) {
        printf("%s", SH_PROMPT);
        fflush(stdout);  /* Ensure prompt is output immediately */
        if (fgets(line, SH_CMD_MAX, stdin) == NULL) {
            printf("\n");
            break;
        }
        /* Remove the trailing newline */
        line[strcspn(line, "\n")] = '\0';

        /* Allocate and build the command buffer for this input */
        cmd_buff_t cmd;
        if (alloc_cmd_buff(&cmd) != OK) {
            fprintf(stderr, "Memory allocation error\n");
            continue;
        }
        if (build_cmd_buff(line, &cmd) == WARN_NO_CMDS) {
            printf(CMD_WARN_NO_CMD);
            free_cmd_buff(&cmd);
            continue;
        }

        /* Check if the command is a built-in */
        Built_In_Cmds bi = match_command(cmd.argv[0]);
        if (bi != BI_NOT_BI) {
            Built_In_Cmds ret = exec_built_in_cmd(&cmd);
            free_cmd_buff(&cmd);
            if (ret == BI_CMD_EXIT) {
                return OK_EXIT;
            }
        } else {
            /* Not a built-in, so fork/exec external command */
            exec_cmd(&cmd);
            free_cmd_buff(&cmd);
        }
    }
    return OK;
}
