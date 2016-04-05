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

int run();
void readline(char*);
void parseline(char**, char*);
int executeargs(char**);
int execute(char**);
int shellcd(char**);
int shellexit(char**);

int (*specialfunc[])(char **) = {&shellcd, &shellexit};
char *special[] = {"cd", "exit"};

int main() {
  int status = run();
  exit(status);
}

int run() {
  char* line;
  char** args;
  int status = CONTINUE;

  while(status == CONTINUE) {
    printf("[mycoolshell]$ ");

    line = malloc(sizeof(char)*MAXLINE);
    readline(line);

    args = malloc(sizeof(char*)*MAXLINE);
    parseline(args, line);

    status = executeargs(args);

    free(line);
    free(args);
  }

  return status;
}

void readline(char *line) {
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

void parseline(char **args, char *line) {
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

int executeargs(char **args) {
  int i;
  for(i = 0; special[i] != NULL; i++) {
    if(strcmp(special[i], args[0]) == 0) {
      return (*specialfunc[i])(args);
    }
  }
  return execute(args);
}

int execute(char **args) {
  int pid = fork();
  int childPid;
  int status;
  
  if(pid == 0) {
    execvp(args[0], args);
    fprintf(stderr, "Could not execute: %s\n", args[0]);
    return EXIT_FAILURE;
  } else {
    childPid = wait(&status);
  }
  return CONTINUE;
}

int shellcd(char **args) {
  if(args[1] != NULL) {
    chdir(args[1]);
  } else {
    chdir(getenv("HOME"));
  }
  return CONTINUE;
}

//Special handling of exit command
int shellexit(char **args) {
  return EXIT_SUCCESS;
}
