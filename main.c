/*
 * Nate Benz
 * CSIS 381 Systems Programming
 * Lab 8
 *
 * This program implements a simple shell that 
 * can pipe and exec commands, cd, and exit
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "defs.h"

void run();
void read_line(char*);
void parse_line(char**, char*);
int parse_commands(char***, char**);

int execute_commands(int, char***);
void execute(char**);
void execute_args(char**);
int run_pipe(int, int, char***);
int shell_cd(char**);
int shell_exit(char**);

int (*special_func[])(char **) = {&shell_cd, &shell_exit};
char *special[] = {"cd", "exit"};

int main() {
  run();
  exit(EXIT_SUCCESS);
}

//Main run loop
void run() {
  char* line;
  char** args;
  char*** commands;

  int status = CONTINUE;
  int i = 0;
  int num_commands;

  while(1) {
    printf("[mycoolshell]$ ");

    line = malloc(sizeof(char)*MAXLINE);
    read_line(line);

    args = malloc(sizeof(char*)*MAXLINE);
    parse_line(args, line);

    commands = malloc(sizeof(char**)*MAXLINE);
    num_commands = parse_commands(commands, args);

    /* Ideally this shouldn't take so many methods to handle,
     * but handling the built-ins wasn't playing nice with the pipes.
     * Still working on DRYing this up.
     */

    if(num_commands == 1)
      execute_args(commands[0]); //Use standard argument handler
    else                             
      execute_commands(num_commands, commands); //Use piping routine

    free(line);
    free(args);
    free(commands);
  }
}

//Reads in a line from stdin
void read_line(char *line) {
  int c, i;
  int length = MAXLINE;

  i = 0;
  while(1) {
    if(i == length) {
      length += MAXLINE;
      line = realloc(line, length);
      if(!line) {
        printf("Error allocating space of size: %d\n", length);
        exit(EXIT_FAILURE); 
      }
    }

    c = getch();
    if(c == EOF || c == '\n') {
      line[i++] = '\0';
      return;
    } else {
      line[i++] = c;
    }
  }
}

//Splits line in to arguments (delimited by whitespace)
void parse_line(char **args, char *line) {
  int i = 0;
  int length = MAXWORD; 
  char *word = strtok(line, TOKEN_SPLIT);

  args[0] = word;

  while((args[++i] = strtok(NULL, TOKEN_SPLIT)) != NULL) {
    if(i == length) {
      length += MAXWORD;
      args = realloc(args, length*sizeof(char*)); 
      if(!args) {
        printf("Error allocating space of size: %d\n", length);
        exit(EXIT_FAILURE); 
      }
    }
  }
}

//Splits argument list into commands (delimited by vertical pipes | )
int parse_commands(char ***commands, char **args) {
  int i = 0;

  commands[i++] = args;
  while(*args != NULL) {
    if(strcmp(*args, PIPE) == 0) {
      *args = NULL;
      commands[i++] = args + 1;
    }
    *args++;
  }
  return i;
}

//Execute an argument list, checking for built in functions first
void execute_args(char **args) {
  int i;
  for(i = 0; special[i] != NULL; i++) {
    if(strcmp(special[i], args[0]) == 0) {
      (*special_func[i])(args);
      return;
    }
  }
  execute(args);
}

//Execute arguments that can be execvp'd
void execute(char **args) {
  int status;

  if(fork() == 0) {
    execvp(args[0], args);
    fprintf(stderr, "Could not execute: %s\n", args[0]);
    exit(EXIT_FAILURE);
  } else {
    wait(&status);
  }
}

//Execute a command list that needs to be piped
int execute_commands(int n, char ***commands) {
  int i;
  int in, fd[2];
  int status;

  if(fork() == 0) {
    in = 0;

    for(i = 0; i < n-1; i++) {
      pipe(fd);
      status = run_pipe(in, fd[WRITE], commands + i);
      close(fd[WRITE]);
      in = fd[READ]; //Save read end for next command
    }

    if(in != READ) {
      dup2(in, READ);
    }

    execvp(commands[i][0], commands[i]); //Execute the last command, printing to stdout
    fprintf(stderr, "Could not execute: %s\n", *commands[0]);
    exit(EXIT_FAILURE);
  } else {
    wait(&status);
  }
}

//Execute arguments on a pipe
int run_pipe(int in, int out, char ***commands) {
  int status;
  int pid = fork();

  if(pid == 0) {
    if(in != READ) {
      dup2(in, READ);
      close(in);
    }
    if(out != WRITE) {
      dup2(out, WRITE);
      close(out);
    }

    execvp(*commands[0], *commands);
    fprintf(stderr, "Could not execute: %s\n", *commands[0]);
    exit(EXIT_FAILURE);
  } else {
    wait(&status);
  }
  return CONTINUE;
}

//Built in for 'cd'
int shell_cd(char **args) {
  if(args[1] != NULL) {
    chdir(args[1]);
  } else {
    chdir(getenv("HOME"));
  }
  return CONTINUE;
}

//Built in for 'exit'
int shell_exit(char **args) {
  exit(EXIT_SUCCESS);
}
