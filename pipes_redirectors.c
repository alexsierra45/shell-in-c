#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

// Check < value in args
int check_red_in(char **args) {
  for (int i = 0; args[i] != NULL; i++) {
    if (strcmp(args[i], "<") == 0) return i;
  }

  return 0;
}

// Check > or >> value in args
int check_red_out(char **args) {
  for (int i = 0; args[i] != NULL; i++) {
    if (strcmp(args[i], ">") == 0) return i;
    else if (strcmp(args[i], ">>") == 0) return -i;
  }

  return 0;
}

// Check pipes
int pipes(char **args, int fdin, int fdout) {
  for (int i = 0; args[i] != NULL; i++) {
    if (strcmp(args[i], "|") == 0) {
      char **a_bef = arr_cpy(args, i, 1);
      char **a_aft = arr_cpy(args, i+1, 0);
      pid_t pid;
      int fd[2];
      int in = dup(0);
      int out = dup(1);
      pipe(fd);

      pid = fork();
      if (pid == 0) {
        close(fd[0]);
        shell_launch(a_bef, fdin, fd[1]);
        close(fd[1]);
      } else {
        wait(NULL);
        close(fd[1]);
        lsh_execute(a_aft, fd[0], fdout);
        close(fd[0]);
      }

      dup2(in, 0);
      dup2(out, 1);
      close(in);
      close(out);

      return 0;
    }
  }

  return 1;
}