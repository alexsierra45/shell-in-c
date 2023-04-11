#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MY_SH_TOK_DELIM " \t\r\n\a"
#define ERROR "\033[1;31mmy_sh\033[0m"
#define LSH_TOK_BUFSIZE 64

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

// arr copy
char **arr_cpy(char **arr, int i, int bool) {
  int bufsize = LSH_TOK_BUFSIZE, position = 0;
  char **a_c = malloc(bufsize * sizeof(char*));

  if (bool) {
    while (position < i) {
      a_c[position] = arr[position];
      position++;
    }
    a_c[i] = NULL;
  }
  else {
    while (arr[i] != NULL) 
      a_c[position++] = arr[i++];
    a_c[position] = NULL;
  }
  
  return a_c;
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