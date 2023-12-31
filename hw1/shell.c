#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdbool.h>


#define FALSE 0
#define TRUE 1
#define INPUT_STRING_SIZE 80
#define REDIRECT_IN "<"
#define REDIRECT_OUT ">"
#define BACKGROUND "&"

#include "io.h"
#include "parse.h"
#include "process.h"
#include "shell.h"

int cmd_quit(tok_t arg[]) {
    printf("Bye\n");
    exit(0);
    return 1;
}

int cmd_help(tok_t arg[]);

char *create_path(const char *executable_path, tok_t path_variable);

void make_program_path(tok_t *tokens);

void redirect_io(tok_t *tokens);

bool is_space(char *s);

int handle_background_index(tok_t *t);

void put_parent_process_in_background(pid_t child_pid);

int cmd_pwd(tok_t arg[]) {
    size_t size = sizeof(char) * 256;
    char *path = getcwd(NULL, size);
    if (path != NULL)
        printf("%s\n", path);
    return 0;
}

int cmd_cd(tok_t arg[]) {
    if (arg[0] == NULL) {
        return 1;
    }

    int res = chdir(arg[0]);
    if (res != 0)
        return 1;

    return 0;
}

int cmd_wait(tok_t arg[]) {
    int status, wpid;
    while ((wpid = wait(&status)) > 0);
    return 1;
}


/* Command Lookup table */
typedef int cmd_fun_t(tok_t args[]); /* cmd functions take token array and return int */
typedef struct fun_desc {
    cmd_fun_t *fun;
    char *cmd;
    char *doc;
} fun_desc_t;

fun_desc_t cmd_table[] = {
        {cmd_help, "?",    "show this help menu"},
        {cmd_quit, "quit", "quit the command shell"},
        {cmd_pwd,  "pwd",  "get the current working directory"},
        {cmd_cd,   "cd",   "change the current directory to provided one"},
        {cmd_wait, "wait", "wait for child processes to finish"}
};

int cmd_help(tok_t arg[]) {
    int i;
    for (i = 0; i < (sizeof(cmd_table) / sizeof(fun_desc_t)); i++) {
        printf("%s - %s\n", cmd_table[i].cmd, cmd_table[i].doc);
    }
    return 1;
}

int lookup(char cmd[]) {
    int i;
    for (i = 0; i < (sizeof(cmd_table) / sizeof(fun_desc_t)); i++) {
        if (cmd && (strcmp(cmd_table[i].cmd, cmd) == 0)) return i;
    }
    return -1;
}

void deactivate_signals()
{
    signal(SIGTSTP, SIG_IGN);
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
}

void activate_signals()
{
    signal(SIGTSTP, SIG_DFL);
    signal(SIGINT, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    signal(SIGTTIN, SIG_DFL);
    signal(SIGTTOU, SIG_DFL);
    signal(SIGTTOU, SIG_DFL);
}

void init_shell() {
    /* Check if we are running interactively */
    shell_terminal = STDIN_FILENO;

    /** Note that we cannot take control of the terminal if the shell
        is not interactive */
    shell_is_interactive = isatty(shell_terminal);

    if (shell_is_interactive) {

        /* force into foreground */
        while (tcgetpgrp(shell_terminal) != (shell_pgid = getpgrp()))
            kill(-shell_pgid, SIGTTIN);

        shell_pgid = getpid();
        /* Put shell in its own process group */
        if (setpgid(shell_pgid, shell_pgid) < 0) {
            perror("Couldn't put the shell in its own process group");
            exit(1);
        }

        /* Take control of the terminal */
        tcsetpgrp(shell_terminal, shell_pgid);
        tcgetattr(shell_terminal, &shell_tmodes);
        deactivate_signals();
    }
}

/**
 * Add a process to our process list
 */
void add_process(process *p) {
    /** YOUR CODE HERE */
}

/**
 * Creates a process given the inputString from stdin
 */
process *create_process(char *inputString) {
    /** YOUR CODE HERE */
    return NULL;
}


int shell(int argc, char *argv[]) {
    char *s = malloc(INPUT_STRING_SIZE + 1); 
    tok_t *t;           
    int fundex = -1;


    init_shell();
    
    while ((s = freadln(stdin))) {
        if (is_space(s)) {
            continue;
        }
        t = getToks(s); 
        fundex = lookup(t[0]);
        if (fundex >= 0)
            cmd_table[fundex].fun(&t[1]);
        else {
            int back_index = handle_background_index(t);
            pid_t child_pid = fork();
            if (child_pid < 0) {
                continue;
            } else if (child_pid == 0) {
                if (back_index == 0) {
                    activate_signals();
                }
                redirect_io(t);
                make_program_path(t);
                execv(t[0], t);
                exit(0);
            } else {
                if (back_index == 0) {
                    put_parent_process_in_background(child_pid);
                }
            }
        }
    }

    return 0;
}

void put_parent_process_in_background(pid_t child_pid) {
    deactivate_signals();
    setpgid(child_pid,child_pid);
    tcsetpgrp(STDIN_FILENO, child_pid);
    int status;
    waitpid(child_pid, &status, WUNTRACED);
    tcsetpgrp(STDIN_FILENO, shell_pgid);
}

int handle_background_index(tok_t *t) {
    int back_index = isDirectTok(t, BACKGROUND);
    if (back_index > 0)
        t[back_index] = NULL;
    return back_index;
}

bool is_space(char *s) {
    for (int i = 0; i < strlen(s); ++i) {
        if (!isspace(s[i])) {
            return FALSE;
        }
    }
    return TRUE;
}

void redirect_io(tok_t *tokens) {
    int in_index = isDirectTok(tokens, REDIRECT_IN);
    if (in_index > 0) {
        int in_file_desc = open(tokens[in_index + 1], O_RDONLY);
        dup2(in_file_desc, STDIN_FILENO);
        tokens[in_index] = tokens[in_index + 1] = NULL;
    }
    int out_index = isDirectTok(tokens, REDIRECT_OUT);
    if (out_index > 0) {
        int file_desc = open(tokens[out_index + 1], O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);
        dup2(file_desc, STDOUT_FILENO);
        tokens[out_index] = tokens[out_index + 1] = NULL;
    }
}

void make_program_path(tok_t *tokens) {
    if (access(tokens[0], F_OK) == 0) {
        return;
    }
    const char *executable_path = tokens[0];
    char *environment_path = getenv("PATH");
    tok_t *path_variables = getToks(environment_path);
    for (int i = 0; i < MAXTOKS - 1 && path_variables[i]; ++i) {
        char *test_path = create_path(executable_path, path_variables[i]);
        if (access(test_path, F_OK) == 0) {
            tokens[0] = test_path;
            return;
        }
    }
}

char *create_path(const char *executable_path, tok_t path_variable) {
    char *real_path = (char *) malloc(sizeof(char) * MAXLINE);
    strcat(real_path, path_variable);
    strcat(real_path, "/");
    strcat(real_path, executable_path);
    strcat(real_path, "\0");
    return real_path;
}

