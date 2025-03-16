#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "dshlib.h"

/*
 *  build_cmd_list
 *    cmd_line:     the command line from the user
 *    clist *:      pointer to clist structure to be populated
 *
 *  This function builds the command_list_t structure passed by the caller
 *  It does this by first splitting the cmd_line into commands by spltting
 *  the string based on any pipe characters '|'.  It then traverses each
 *  command.  For each command (a substring of cmd_line), it then parses
 *  that command by taking the first token as the executable name, and
 *  then the remaining tokens as the arguments.
 *
 *  NOTE your implementation should be able to handle properly removing
 *  leading and trailing spaces!
 *
 *  errors returned:
 *
 *    OK:                      No Error
 *    ERR_TOO_MANY_COMMANDS:   There is a limit of CMD_MAX (see dshlib.h)
 *                             commands.
 *    ERR_CMD_OR_ARGS_TOO_BIG: One of the commands provided by the user
 *                             was larger than allowed, either the
 *                             executable name, or the arg string.
 *
 *  Standard Library Functions You Might Want To Consider Using
 *      memset(), strcmp(), strcpy(), strtok(), strlen(), strchr()
 */

int build_cmd_list(char *cmd_line, command_list_t *clist)
{
    clist->num = 0;
    char *token = strtok(cmd_line, PIPE_STRING);
    //printf("%s", token);
    while (token != NULL) {
        if (clist->num >= 8) {
            return ERR_TOO_MANY_COMMANDS;
        }
        
        clist->commands[clist->num].args[0] = '\0';

        if (strlen(token) >= EXE_MAX) {
            return ERR_CMD_OR_ARGS_TOO_BIG;
        }

        int i = 0;
        char *buff = (char *) malloc(100);
        int buff_index = 0;
        int counter = 0;
        // printf("%s\n", token);

        while(token[i] != '\0') {
            if (!(isspace(token[i]))) {
                buff[buff_index++] = token[i];
            } else {
                if (buff_index > 0) {
                    buff[buff_index] = '\0';
                    
                    if (counter == 0) {  
                        strcpy(clist->commands[clist->num].exe, buff);
                    } else {
                        if (strlen(clist->commands[clist->num].args) + strlen(buff) + 1 < ARG_MAX) {
                            if (counter > 1) {
                                strcat(clist->commands[clist->num].args, " ");
                            }
                            strcat(clist->commands[clist->num].args, buff);
                        }
                    }
                    buff_index = 0;
                    counter++;
                }
            }
            i++;
        }

        if (buff_index > 0) {
            buff[buff_index] = '\0';
            if (counter == 0) {
                strcpy(clist->commands[clist->num].exe, buff);
            } else {
                if (strlen(clist->commands[clist->num].args) + strlen(buff) + 1 < ARG_MAX) {
                    if (counter > 1) {  
                        strcat(clist->commands[clist->num].args, " ");
                    }
                    strcat(clist->commands[clist->num].args, buff);
                }
            }
        }

        free(buff);


        // char *command = strtok(token, " ");
        // strcpy(clist->commands[clist->num].exe, command);
        // int arg_counter = 0;
        // char *arg = strtok(NULL, " ");
        // while (arg != NULL && arg_counter < ARG_MAX) {
        //     strcpy(clist->commands[clist->num].args, arg);
        //     arg = strtok(NULL, " ");
        //     arg_counter++;
        // }
        // if (arg_counter == 0 && arg == NULL) {
        //     clist->commands[clist->num].args[0] = '\0';
        // }
        
        // For debugging, print the token
        //printf("Token for command %d: %s\n", clist->num + 1, token);
        

        clist->num++; 
        token = strtok(NULL, PIPE_STRING);
    }
    
    return OK;
}
