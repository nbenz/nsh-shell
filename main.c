/*
 * Nate Benz
 * CSIS 381 Systems Programming
 * Lab 8
 *
 * This program implements a simple shell that 
 * can fork and exec commands, cd, and exit
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
int pipe_execute(int, int, char***);
int shell_cd(char**);
int shell_exit(char**);

int (*special_func[])(char **) = {&shell_cd, &shell_exit};
char *special[] = {"cd", "exit"};

int main() {
  run();
  exit(EXIT_SUCCESS);
}

void run() {
  char* line;
  char** args;
  char*** cmds;

  int status = CONTINUE;
  int i = 0;
  int num_cmds;

  while(1) {
    printf("[mycoolshell]$ ");

    line = malloc(sizeof(char)*MAXLINE);
    read_line(line);

    args = malloc(sizeof(char*)*MAXLINE);
    parse_line(args, line);

    cmds = malloc(sizeof(char**)*MAXLINE);
    num_cmds = parse_commands(cmds, args);

    /* status = execute_args(args); */
    
    execute_commands(num_cmds, cmds);

    free(line);
    free(args);
    free(cmds);
  }
}

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

int parse_commands(char ***cmds, char **args) {
  int i = 0;

  cmds[i++] = args;
  while(*args != NULL) {
    if(strcmp(*args, PIPE) == 0) {
      *args = NULL;
      cmds[i++] = args + 1;
    }
    *args++;
  }
  return i;
}

void execute(char **args) {
  int i;
  for(i = 0; special[i] != NULL; i++) {
    if(strcmp(special[i], args[0]) == 0) {
      (*special_func[i])(args);
      return;
    }
  }
  execvp(args[0], args);
  fprintf(stderr, "Could not execute: %s\n", args[0]);
  exit(EXIT_FAILURE);
}

int execute_commands(int n, char ***cmds) {
  int i;
  int in, fd[2];
  int pid = fork();
  int status;

  if(pid == 0) {
    in = 0;

    for(i = 0; i < n-1; i++) {
      pipe(fd);
      status = pipe_execute(in, fd[WRITE], cmds + i);
      close(fd[WRITE]);
      in = fd[READ];
    }

    if(in != 0) {
      dup2(in, 0);
    }

    execute(cmds[i]);
  } else {
    wait(&status);
  }
}

int pipe_execute(int in, int out, char ***cmds) {
  int status;
  int pid = fork();

  if(pid == 0) {
    if(in != 0) {
      dup2(in, 0);
      close(in);
    }
    if(out != 1) {
      dup2(out, 1);
      close(out);
    }

    execute(*cmds);
  } else {
    wait(&status);
  }
  return CONTINUE;
}

int shell_cd(char **args) {
  if(args[1] != NULL) {
    chdir(args[1]);
  } else {
    chdir(getenv("HOME"));
  }
  return CONTINUE;
}

int shell_exit(char **args) {
  exit(EXIT_SUCCESS);
}
