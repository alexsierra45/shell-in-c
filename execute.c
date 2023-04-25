#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "builtin.c"

int execute(char **args, int fdin, int fdout);

// Launch a program.
int shell_launch(char **args, int fdin, int fdout) {
  dup2(fdin, 0);
  dup2(fdout, 1);
  
  if (execvp(args[0], args) == -1) {
    printf("Command not found\n");
    return -1;
  }
}

// Check < value in args
int red_in(char **args, int fdin, int fdout) {
  for (int i = 0; args[i] != NULL; i++) {
    if (strcmp(args[i], "<") == 0) {
      args[i] = NULL;
      int fd = open(args[i+1], O_RDWR | O_CREAT, 0644);
      execute(args, fd, fdout);
      
      return 0;
    }
  }

  return 1;
}

// Check > or >> value in args
int red_out(char **args, int fdin, int fdout) {
  for (int i = 0; args[i] != NULL; i++) {
    if (strcmp(args[i], ">") == 0) {
      if (i == 0) {
        FILE *f = fopen(args[1], "w");
        fclose(f);
      }
      else {
        args[i] = NULL;
        int fd = open(args[i+1], O_RDWR | O_CREAT, 0644);
        execute(args, fdin, fd);
      }
      
      return 0;
    }
    else if (strcmp(args[i], ">>") == 0) {
      args[i] = NULL;
      int fd = open(args[i+1], O_RDWR | O_CREAT | O_APPEND, 0644);
      execute(args, fdin, fd);

      return 0;
    }
  }

  return 1;
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
        execute(a_aft, fd[0], fdout);
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

// Chain commands

// Chain ;
int chain(char **args, int fdin, int fdout) {
  for (int i = 0; args[i] != NULL; i++) {
    if (strcmp(args[i], ";") == 0) {
      char **a_bef = arr_cpy(args, i, 1);
      char **a_aft = arr_cpy(args, i+1, 0);
      execute(a_bef, fdin, fdout);
      execute(a_aft, fdin, fdout);
      return 0;
    }
  }

  return 1;
}

// Chain &&
int chain_and(char **args, int fdin, int fdout) {
  for (int i = 0; args[i] != NULL; i++) {
    if (strcmp(args[i], "&&") == 0) {
      char **a_bef = arr_cpy(args, i, 1);
      char **a_aft = arr_cpy(args, i+1, 0);
      if (execute(a_bef, fdin, fdout) == 0) execute(a_aft, fdin, fdout);
      return 0;
    }
  }

  return 1;
}

// Chain ||
int chain_or(char **args, int fdin, int fdout) {
  for (int i = 0; args[i] != NULL; i++) {
    if (strcmp(args[i], "||") == 0) {
      char **a_bef = arr_cpy(args, i, 1);
      char **a_aft = arr_cpy(args, i+1, 0);
      if (execute(a_bef, fdin, fdout) != 0) execute(a_aft, fdin, fdout);
      return 0;
    }
  }

  return 1;
}

// Conditional execution
int conditions(char **args, int fdin, int fdout) {
  if (strcmp(args[0], "if") == 0) {
    int i = 1;
    args = arr_cpy(args, i, 0);
    i = 0;
    while (strcmp(args[i], "then") != 0) i++;
    char **a_if = arr_cpy(args, i, 1);
    args = arr_cpy(args, i+1, 0);
    i = 0;
    while (strcmp(args[i], "else") != 0) {
      i++;
      if (args[i] == NULL) {
        char **a_then = arr_cpy(args, i-1, 1);
        if (execute(a_if, fdin, fdout) == 0) execute(a_then, fdin, fdout);

        return 0;
      }
    }
    char **a_then = arr_cpy(args, i, 1);
    args = arr_cpy(args, i+1, 0);
    i = 0;
    while (strcmp(args[i], "end") != 0) i++;
    char **a_else = arr_cpy(args, i, 1);
    if (execute(a_if, fdin, fdout) == 0) execute(a_then, fdin, fdout);
    else execute(a_else, fdin, fdout);

    return 0;
  }

  return 1;
}

// Check background
int background(char **args, int fdin, int fdout) {
  for (int i = 0; args[i] != NULL; i++) {
    if (strcmp(args[i], "&") == 0) {
      args[i] = NULL;
      int pid = fork();
      if (pid == 0) {
        setpgid(0, 0);
        execute(args, fdin, fdout);
        exit(0);
      }
      if (pid > 0) {
        setpgid(pid, pid);
        char *path = home_dir("jobs.txt");
        FILE *f = fopen(path, "a");
        char *command = concat_array(args, ' ');
        fprintf(f, "%d %s\n", pid, command);
        fclose(f);
        printf("[%d]\t%d\n", count_lines(path), pid);
        if (args[i+1] != NULL) {
          char **a_aft = arr_cpy(args, i+1, 0);
          execute(a_aft, fdin, fdout);
        }
      }
      
      return 0;
    }
  }

  return 1;
}

// Check set
int set(char **args, int fdin, int fdout) {
  char buffer[1024];
  char *var_dir = home_dir("variables.txt");

  if (strcmp(args[0], "set") == 0) {
    if (args[1] == NULL) {
      FILE *f = fopen(var_dir, "r");
      char line[128];
      while (fgets(line, 128, f) != NULL) {
        if (line[0] == '*') continue;
        printf("%s", line);
      }

      return 0;
    }
    else if (args[2] == NULL) {
      printf("set: not enough arguments\n");
      return 0;
    }
    else {
      int last = 2;
      char **a = arr_cpy(args, 2, 0);
      while (args[last+1] != NULL) last++;
      if (args[2][0] == '`' && args[last][strlen(args[last])-1] == '`') {
        if (last == 2) 
          a[0] = sub_str(a[0], 1, strlen(a[0])-2);
        else {
          a[0] = sub_str(a[0], 1, strlen(a[0])-1);
          a[last-2] = sub_str(a[last-2], 0, strlen(a[last-2])-2);
        }

        char *output_dir = home_dir("output.txt");
        int fd = open(output_dir, O_CREAT | O_WRONLY | O_TRUNC, 0644);
        execute(a, fdin, fd);
        close(fd);
        fd = open(output_dir, O_RDONLY);
        read(fd, buffer, 1024);
        buffer[count_chars(output_dir)] = '*';
        buffer[count_chars(output_dir)+1] = '\0';
        close(fd);
      }
      else {
        char *command = concat_array(a, ' ');
        for (int i = 0; i < strlen(command); i++)
          buffer[i] = command[i];
        buffer[strlen(command)] = '\n';
        buffer[strlen(command)+1] = '*';
        buffer[strlen(command)+2] = '\0';
      }

      char *var = args[1];
      delete_var(var);
      FILE *f = fopen(var_dir, "a");
      fprintf(f, "%s = %s\n", var, buffer);
      fclose(f);

      return 0;
    }
  }

  return 1;
}

int (*operators[]) (char **, int, int) = {
  &set,
  &background,
  &chain,
  &conditions,
  &chain_and,
  &chain_or,
  &pipes,
  &red_in,
  &red_out
};

int lsh_num_operators() {
  return sizeof(operators) / sizeof(char *);
}

// Execute shell built-in or launch program.
int execute(char **args, int fdin, int fdout) {
  if (args[0] == NULL) {
    printf("An empty command was entered, don't be a fool.\n");
    return -1;
  }
  // Search for &, ;, &&, ||, conditions, |, <, >, >>, and execute them
  for (int i = 0; i < lsh_num_operators(); i++) {
    if ((*operators[i])(args, fdin, fdout) == 0) 
      return 1;
  }
  for (int i = 0; i < lsh_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }
  int pid = fork();
  if (pid == 0) {
    if (shell_launch(args, fdin, fdout) == -1)
      exit(-1);
  }
  else {
    int status;
    wait(&status);
    if (WIFEXITED(status)) {
      int exit_status = WEXITSTATUS(status);
      if (exit_status == 255) 
        return -1;
    }
  }
  
  return 1;
}