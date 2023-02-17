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


#define FALSE 0
#define TRUE 1
#define INPUT_STRING_SIZE 80

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

int cmd_pwd(tok_t arg[]) {
    size_t size = sizeof(char) * 256;
    char *path = getcwd(NULL, size);
    if (path != NULL)
        printf("%s", path);
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




/* Command Lookup table */
typedef int cmd_fun_t (tok_t args[]); /* cmd functions take token array and return int */
typedef struct fun_desc {
  cmd_fun_t *fun;
  char *cmd;
  char *doc;
} fun_desc_t;

fun_desc_t cmd_table[] = {
        {cmd_help, "?",    "show this help menu"},
        {cmd_quit, "quit", "quit the command shell"},
        {cmd_pwd,  "pwd",  "get the current working directory"},
        {cmd_cd,   "cd",   "change the current directory to provided one"}
};

int cmd_help(tok_t arg[]) {
  int i;
  for (i=0; i < (sizeof(cmd_table)/sizeof(fun_desc_t)); i++) {
    printf("%s - %s\n",cmd_table[i].cmd, cmd_table[i].doc);
  }
  return 1;
}

int lookup(char cmd[]) {
  int i;
  for (i=0; i < (sizeof(cmd_table)/sizeof(fun_desc_t)); i++) {
    if (cmd && (strcmp(cmd_table[i].cmd, cmd) == 0)) return i;
  }
  return -1;
}

void init_shell()
{
  /* Check if we are running interactively */
  shell_terminal = STDIN_FILENO;

  /** Note that we cannot take control of the terminal if the shell
      is not interactive */
  shell_is_interactive = isatty(shell_terminal);

  if(shell_is_interactive){

    /* force into foreground */
    while(tcgetpgrp (shell_terminal) != (shell_pgid = getpgrp()))
      kill( - shell_pgid, SIGTTIN);

    shell_pgid = getpid();
    /* Put shell in its own process group */
    if(setpgid(shell_pgid, shell_pgid) < 0){
      perror("Couldn't put the shell in its own process group");
      exit(1);
    }

    /* Take control of the terminal */
    tcsetpgrp(shell_terminal, shell_pgid);
    tcgetattr(shell_terminal, &shell_tmodes);
  }
  /** YOUR CODE HERE */
}

/**
 * Add a process to our process list
 */
void add_process(process* p)
{
  /** YOUR CODE HERE */
}

/**
 * Creates a process given the inputString from stdin
 */
process* create_process(char* inputString)
{
  /** YOUR CODE HERE */
  return NULL;
}



int shell (int argc, char *argv[]) {
  char *s = malloc(INPUT_STRING_SIZE+1);			/* user input string */
  tok_t *t;			/* tokens parsed from input */
  int lineNum = 0;
  int fundex = -1;
  pid_t pid = getpid();		/* get current processes PID */
  pid_t ppid = getppid();	/* get parents PID */
  pid_t cpid, tcpid, cpgid;

  init_shell();

 // printf("%s running as PID %d under %d\n",argv[0],pid,ppid);

  lineNum=0;
  // fprintf(stdout, "%d: ", lineNum);
        while ((s = freadln(stdin))) {
        t = getToks(s); /* break the line into tokens */
        fundex = lookup(t[0]); /* Is first token a shell literal */
        if (fundex >= 0)
            cmd_table[fundex].fun(&t[1]);
        else {
            pid_t child_pid = fork();
            if (child_pid < 0) {
                continue;
            } else if (child_pid == 0) {
            	for (int i = 1; t[i] && i < MAXTOKS; ++i) {
                    if (strcmp(t[i], "<") == 0) {
                        FILE *infile = fopen(t[i+1], "r");
                        char *new_arg = (char *) malloc(sizeof(char) * 128);
                        int j = i;
                        while (fscanf(infile, "%s", new_arg) != EOF)  {
                            t[j] = new_arg;
                            j++;
                        }
                        t[j] = NULL;
                        break;
                    } else if (strcmp(t[i], ">") == 0) {
                        printf("file >: %s", t[i+1]);
                        int file_desc = open(t[i+1], O_CREAT | O_RDWR, S_IRWXU);
                        dup2(file_desc, 1) ;
                        t[i] = NULL;
                        break;
                    }
                }
                
		execv(t[0], t);
                const char *executable_path = t[0];
                char *path = getenv("PATH");
                tok_t *path_variables = getToks(path);
                char *real_path = (char *) malloc(sizeof(char) * 1024);
                for (int i = 0; i < MAXTOKS; ++i) {
                    strcat(real_path, path_variables[i]);
                    strcat(real_path, "/");
                    strcat(real_path, executable_path);
                    strcat(real_path, "\0");
                    t[0] = real_path;
                    execv(t[0], t);
                    memset(real_path, 0, sizeof(char) * 1024);
                }
                exit(EXIT_SUCCESS);
            } else {
                int status;
                waitpid(child_pid, &status, 0);
            }
        }
        // fprintf(stdout, "%d: ", lineNum);
    }

  return 0;
}
