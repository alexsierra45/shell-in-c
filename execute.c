#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MY_SH_TOK_DELIM " \t\r\n\a"
#define ERROR "\033[1;31mmy_sh\033[0m"
#define LSH_TOK_BUFSIZE 64

int lsh_execute(char **args);

// arr copy
char **arr_cpy(char **arr, int i, int bool) {
  int bufsize = LSH_TOK_BUFSIZE, position = 0;
  char **a_c = malloc(bufsize * sizeof(char*));

  if (bool) {
    while (position < i) 
      a_c[position] = arr[position++];
    a_c[i] = NULL;
  }
  else {
    while (arr[i] != NULL) 
      a_c[position++] = arr[i++];
    a_c[position] = NULL;
  }
  
  return a_c;
}

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
  &lsh_cd,
  &lsh_help,
  &lsh_exit
};

int lsh_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

// Builtin function implementations.

// Change directory.
int lsh_cd(char **args) {
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
int lsh_help(char **args) {
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
int lsh_exit(char **args) {
  return 0;
}

// Launch a program and wait for it to terminate.
int lsh_launch(char **args) {
  pid_t pid, wpid;
  int status;

  pid = fork();
  if (pid == 0) {
    // Child process
    if (execvp(args[0], args) == -1) {
      perror("lsh");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    // Error forking
    perror("lsh");
  } else {
    // Parent process
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

int check_pipes(char **args) {
}

// Redirect output to file.
int redirect_out(char *fileName, int fd) {
    int fd2 = open(fileName, O_RDONLY);
    if (fd2 == -1) {
        perror("open");
        return 1;
    }
    if (dup2(fd2, fd) == -1) {
        perror("dup2");
        return 1;
    }
    if (close(fd2) == -1) {
        perror("close");
        return 1;
    }
    return 0;
}

// Redirect input from file.
int redirect_in(char *fileName, int fd) {
    int fd2 = open(fileName, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd2 == -1) {
        perror("open");
        return 1;
    }
    if (dup2(fd2, fd) == -1) {
        perror("dup2");
        return 1;
    }
    if (close(fd2) == -1) {
        perror("close");
        return 1;
    }
    return 0;
}

int check_redirectors(char ** args) {
    int bufsize = LSH_TOK_BUFSIZE, position = 0;
    char **new_arr = malloc(bufsize * sizeof(char*));
    int i = 0;

    while (args[i] != NULL) {
        if (strcmp(args[i], "<") == 0) {
            redirect_in(args[i+1], STDIN_FILENO);
            int pid = fork();
            if (pid == 0) {
                execvp(args[i-1], args);
            }
            else if (pid > 0) {
                wait(NULL);
            }
            else {
                printf("Error al crear el proceso hijo\n");
                return 1;
            }
        }
        else if (strcmp(args[i], ">") == 0) {
            redirect_out(args[i+1], STDOUT_FILENO);
        }
        i++;
        new_arr[position++] = args[i];
    }
}

// Execute shell built-in or launch program.
int lsh_execute(char **args) {
  int i;

  if (args[0] == NULL) {
    printf("An empty command was entered, don't be a fool.\n");
    return 1;
  }
  check_redirectors(args);
  // check_pipes(args);
  for (i = 0; i < lsh_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  return lsh_launch(args);
}