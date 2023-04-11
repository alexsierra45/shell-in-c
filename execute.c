#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "pipes_redirectors.c"

int lsh_execute(char **args, int fdin, int fdout);

// Function Declarations for builtin shell commands:
int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);

// List of builtin commands, followed by their corresponding functions.
char *builtin_str[] = {
  "cd",
  "help",
  "exit"
};

int (*builtin_func[]) (char **) = {
  &shell_cd,
  &shell_help,
  &shell_exit
};

int lsh_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

// Builtin function implementations.

// Change directory.
int shell_cd(char **args) {
  if (args[1] == NULL) {
    fprintf(stderr, "lsh: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("lsh");
    }
  }
  return 1;
}

// Help builtin command.
int shell_help(char **args) {
  int i;
  printf("Shell-in-c's LSH\n");
  printf("Type program names and arguments, and hit enter.\n");
  printf("The following are built in:\n");

  for (i = 0; i < lsh_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }

  printf("Use the man command for information on other programs.\n");
  return 1;
}

// Exit builtin command.
int shell_exit(char **args) {
  return 0;
}

// Launch a program.
void shell_launch(char **args, int fdin, int fdout) {
  pid_t pid, wpid;
  int status;
  int in = dup(0);
  int out = dup(1);
  dup2(fdin, 0);
  dup2(fdout, 1);
  int redin = check_red_in(args);
  int redout = check_red_out(args);
  if (redin != 0) {
    args[check_red_in(args)] = NULL;
    int fd = open(args[redin+1], O_RDWR | O_CREAT, 0644);
    dup2(fd, 0);
    close(fd);
  }
  if (redout != 0) {
    args[abs(redout)] = NULL;
    int fd;
    if (redout > 0) fd = open(args[redout+1], O_RDWR | O_CREAT, 0644);
    else fd = open(args[-redout+1], O_RDWR | O_CREAT | O_APPEND, 0644);
    dup2(fd, 1);
    close(fd);
  }

  if (execvp(args[0], args) == -1) {
    perror("lsh");
  }
}

// Execute shell built-in or launch program.
int lsh_execute(char **args, int fdin, int fdout) {
  if (args[0] == NULL) {
    printf("An empty command was entered, don't be a fool.\n");
    return 1;
  }
  if (pipes(args, fdin, fdout) != 0) {
    for (int i = 0; i < lsh_num_builtins(); i++) {
      if (strcmp(args[0], builtin_str[i]) == 0) {
        return (*builtin_func[i])(args);
      }
    }

    int pid = fork();
    if (pid == 0) {
      shell_launch(args, fdin, fdout);
    }
    else wait(NULL);
  }
  
  return 1;
}