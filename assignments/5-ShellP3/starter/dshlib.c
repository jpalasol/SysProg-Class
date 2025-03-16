#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include "dshlib.h"

static int last_exit_status = 0;

int alloc_cmd_buff(cmd_buff_t *cmd_buff) {
    cmd_buff->_cmd_buffer = malloc(SH_CMD_MAX);
    if (!cmd_buff->_cmd_buffer)
        return ERR_MEMORY;
    return clear_cmd_buff(cmd_buff);
}

int free_cmd_buff(cmd_buff_t *cmd_buff) {
    if (cmd_buff->_cmd_buffer) {
        free(cmd_buff->_cmd_buffer);
        cmd_buff->_cmd_buffer = NULL;
    }
    return OK;
}

int clear_cmd_buff(cmd_buff_t *cmd_buff) {
    cmd_buff->argc = 0;
    for (int i = 0; i < CMD_ARGV_MAX; i++)
        cmd_buff->argv[i] = NULL;
    cmd_buff->input_redir = NULL;
    cmd_buff->output_redir = NULL;
    cmd_buff->output_append = 0;
    return OK;
}

int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd_buff) {
    strncpy(cmd_buff->_cmd_buffer, cmd_line, SH_CMD_MAX - 1);
    cmd_buff->_cmd_buffer[SH_CMD_MAX - 1] = '\0';

    char *start = cmd_buff->_cmd_buffer;
    while (*start && isspace((unsigned char)*start))
        start++;
    if (*start == '\0')
        return WARN_NO_CMDS;
    
    char *end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) {
        *end = '\0';
        end--;
    }
    
    int token_count = 0;
    char *p = start;
    while (*p && token_count < CMD_ARGV_MAX - 1) {
        while (*p && isspace((unsigned char)*p))
            p++;
        if (*p == '\0')
            break;

        if (*p == '<') {
            p++;
            while (*p && isspace((unsigned char)*p))
                p++;
            if (*p == '\"') {
                p++;
                cmd_buff->input_redir = p;
                while (*p && *p != '\"')
                    p++;
                if (*p == '\"') {
                    *p = '\0';
                    p++;
                }
            } else {
                cmd_buff->input_redir = p;
                while (*p && !isspace((unsigned char)*p))
                    p++;
                if (*p) {
                    *p = '\0';
                    p++;
                }
            }
            continue;
        }
        else if (*p == '>') {
            int append_flag = 0;
            p++;
            if (*p == '>') {
                append_flag = 1;
                p++;
            }
            while (*p && isspace((unsigned char)*p))
                p++;
            if (*p == '\"') {
                p++;
                cmd_buff->output_redir = p;
                while (*p && *p != '\"')
                    p++;
                if (*p == '\"') {
                    *p = '\0';
                    p++;
                }
            } else {
                cmd_buff->output_redir = p;
                while (*p && !isspace((unsigned char)*p))
                    p++;
                if (*p) {
                    *p = '\0';
                    p++;
                }
            }
            cmd_buff->output_append = append_flag;
            continue;
        }
        else if (*p == '\"') {
            p++;
            char *token = p;
            while (*p && *p != '\"')
                p++;
            if (*p == '\"') {
                *p = '\0';
                p++;
            }
            cmd_buff->argv[token_count++] = token;
        }
        else {
            char *token = p;
            while (*p && !isspace((unsigned char)*p) && *p != '<' && *p != '>')
                p++;
            if (*p) {
                *p = '\0';
                p++;
            }
            cmd_buff->argv[token_count++] = token;
        }
    }
    cmd_buff->argv[token_count] = NULL;
    cmd_buff->argc = token_count;
    return (token_count == 0) ? WARN_NO_CMDS : OK;
}

int build_cmd_list(char *cmd_line, command_list_t *clist) {
    clist->num = 0;
    char *saveptr;
    char *token = strtok_r(cmd_line, PIPE_STRING, &saveptr);
    while (token != NULL) {
        while (*token && isspace((unsigned char)*token))
            token++;
        if (*token == '\0') {
            token = strtok_r(NULL, PIPE_STRING, &saveptr);
            continue;
        }
        if (clist->num >= CMD_MAX)
            return ERR_TOO_MANY_COMMANDS;
        if (alloc_cmd_buff(&clist->commands[clist->num]) != OK)
            return ERR_MEMORY;
        int ret = build_cmd_buff(token, &clist->commands[clist->num]);
        if (ret != OK) {
            free_cmd_buff(&clist->commands[clist->num]);
            token = strtok_r(NULL, PIPE_STRING, &saveptr);
            continue;
        }
        clist->num++;
        token = strtok_r(NULL, PIPE_STRING, &saveptr);
    }
    return (clist->num == 0) ? WARN_NO_CMDS : OK;
}

int free_cmd_list(command_list_t *clist) {
    for (int i = 0; i < clist->num; i++) {
        free_cmd_buff(&clist->commands[i]);
    }
    clist->num = 0;
    return OK;
}

Built_In_Cmds match_command(const char *input) {
    if (strcmp(input, EXIT_CMD) == 0)
        return BI_CMD_EXIT;
    if (strcmp(input, "cd") == 0)
        return BI_CMD_CD;
    if (strcmp(input, "dragon") == 0)
        return BI_CMD_DRAGON;
    return BI_NOT_BI;
}

Built_In_Cmds exec_built_in_cmd(cmd_buff_t *cmd) {
    Built_In_Cmds bi = match_command(cmd->argv[0]);
    if (bi == BI_CMD_EXIT) {
        return BI_CMD_EXIT;
    } else if (bi == BI_CMD_CD) {
        if (cmd->argc < 2) {
        } else {
            if (chdir(cmd->argv[1]) != 0)
                fprintf(stderr, "cd: error: cannot change directory to %s\n", cmd->argv[1]);
        }
    } else if (bi == BI_CMD_DRAGON) {
        printf("The requested operation is not implemented yet!\n");
    }
    return BI_EXECUTED;
}

int execute_pipeline(command_list_t *clist) {
    int num_cmds = clist->num;
    int i, pipefd[2], prev_fd = -1;
    pid_t pids[CMD_MAX];
    
    for (i = 0; i < num_cmds; i++) {
        if (i < num_cmds - 1) {
            if (pipe(pipefd) < 0) {
                perror("pipe");
                return ERR_EXEC_CMD;
            }
        }
        pids[i] = fork();
        if (pids[i] < 0) {
            perror("fork");
            return ERR_EXEC_CMD;
        } else if (pids[i] == 0) {
            if (prev_fd != -1) {
                if (dup2(prev_fd, STDIN_FILENO) < 0) {
                    perror("dup2");
                    exit(ERR_EXEC_CMD);
                }
                close(prev_fd);
            }
            if (i < num_cmds - 1) {
                close(pipefd[0]);
                if (dup2(pipefd[1], STDOUT_FILENO) < 0) {
                    perror("dup2");
                    exit(ERR_EXEC_CMD);
                }
                close(pipefd[1]);
            }
            if (clist->commands[i].input_redir) {
                int fd_in = open(clist->commands[i].input_redir, O_RDONLY);
                if (fd_in < 0) {
                    perror("open input");
                    exit(ERR_EXEC_CMD);
                }
                if (dup2(fd_in, STDIN_FILENO) < 0) {
                    perror("dup2 input");
                    exit(ERR_EXEC_CMD);
                }
                close(fd_in);
            }
            if (clist->commands[i].output_redir) {
                int flags = O_WRONLY | O_CREAT;
                flags |= (clist->commands[i].output_append ? O_APPEND : O_TRUNC);
                int fd_out = open(clist->commands[i].output_redir, flags, 0644);
                if (fd_out < 0) {
                    perror("open output");
                    exit(ERR_EXEC_CMD);
                }
                if (dup2(fd_out, STDOUT_FILENO) < 0) {
                    perror("dup2 output");
                    exit(ERR_EXEC_CMD);
                }
                close(fd_out);
            }
            execvp(clist->commands[i].argv[0], clist->commands[i].argv);
            fprintf(stderr, "execvp: %s\n", strerror(errno));
            exit(ERR_EXEC_CMD);
        } else {
            if (prev_fd != -1)
                close(prev_fd);
            if (i < num_cmds - 1) {
                close(pipefd[1]);
                prev_fd = pipefd[0];
            }
        }
    }
    for (i = 0; i < num_cmds; i++) {
        int wstatus;
        waitpid(pids[i], &wstatus, 0);
        if (i == num_cmds - 1) {
            if (WIFEXITED(wstatus))
                last_exit_status = WEXITSTATUS(wstatus);
            else
                last_exit_status = -1;
        }
    }
    return OK;
}

int exec_local_cmd_loop() {
    char line[SH_CMD_MAX];
    command_list_t clist;
    int command_executed = 0;
    
    while (1) {
        if (isatty(STDIN_FILENO)) {
            printf("%s", SH_PROMPT);
            fflush(stdout);
        }
        
        if (fgets(line, sizeof(line), stdin) == NULL) {
            if (command_executed) {
                printf("%s", SH_PROMPT);
                fflush(stdout);
            }
            printf("\n");
            break;
        }
        line[strcspn(line, "\n")] = '\0';
        if (strcmp(line, EXIT_CMD) == 0)
            break;
        if (strlen(line) == 0) {
            printf(CMD_WARN_NO_CMD);
            continue;
        }
        int parse_result = build_cmd_list(line, &clist);
        if (parse_result != OK) {
            if (parse_result == WARN_NO_CMDS)
                printf(CMD_WARN_NO_CMD);
            else if (parse_result == ERR_TOO_MANY_COMMANDS)
                printf(CMD_ERR_PIPE_LIMIT, CMD_MAX);
            else
                printf("Error: Command parsing failed with code %d\n", parse_result);
            continue;
        }
        if (clist.num == 1) {
            Built_In_Cmds bi = match_command(clist.commands[0].argv[0]);
            if (bi != BI_NOT_BI) {
                Built_In_Cmds ret = exec_built_in_cmd(&clist.commands[0]);
                free_cmd_list(&clist);
                if (ret == BI_CMD_EXIT)
                    return OK_EXIT;
                printf("%s", SH_PROMPT);
                fflush(stdout);
                command_executed = 1;
                continue;
            }
        }
        int exec_result = execute_pipeline(&clist);
        if (exec_result != OK)
            printf(CMD_ERR_EXECUTE);
        free_cmd_list(&clist);
        command_executed = 1;
        printf("%s", SH_PROMPT);
        fflush(stdout);
    }
    return OK;
}
