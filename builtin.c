#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <dirent.h>
#include "help_functions.c"

// Function Declarations for builtin shell commands:
int shell_cd(char **args);
int shell_help(char **args);
int shell_exit(char **args);
int shell_true(char **args);
int shell_false(char **args);
int shell_jobs(char **args);
int shell_fg(char ** args);
int shell_history(char **args);
int shell_unset(char **args);
int shell_get(char **args);

// List of builtin commands, followed by their corresponding functions.
char *builtin_str[] = {
  "cd",
  "help",
  "exit",
  "true",
  "false",
  "jobs",
  "fg",
  "history",
  "unset",
  "get"
};

int (*builtin_func[]) (char **) = {
  &shell_cd,
  &shell_help,
  &shell_exit, 
  &shell_true, 
  &shell_false,
  &shell_jobs,
  &shell_fg,
  &shell_history,
  &shell_unset,
  &shell_get
};

int lsh_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

// Builtin function implementations.

// Change directory bultin command.
int shell_cd(char **args) {
  if (args[1] == NULL) {
    char *home_dir = getenv("HOME");
    int ret = chdir(home_dir);
  } else {
    if (chdir(args[1]) != 0) {
      perror("lsh");
    }
  }
  return 1;
}

// Help builtin command.
int shell_help(char **args) {
  char *buf = malloc(1000);
  ssize_t bytes_read;
  if (args[1] == NULL) {
    int fd = open("help/general.txt", O_RDONLY);
    ssize_t bytes_read;
    while ((bytes_read = read(fd, buf, 1000)) > 0) {
      write(1, buf, bytes_read);
    }
    free(buf);
    close(fd);
  }
  else {
    char *cmd_name = args[1];
    char help[100] = "help/";
    char *new_str = strcat(help, strcat(cmd_name, ".txt"));
    int fd = open(new_str, O_RDONLY);
    if (fd < 0) {
      int size;
      for (int i = 0; i < 100; i++) {
        if (cmd_name[i] == '.') {
          size = i;
          break;
        }
      }
      char *new_args[3] = {sub_str(cmd_name, 0, size-1), "--help", NULL};
      int pid = fork();
      if (pid == 0) execvp(new_args[0], new_args);
      else wait(NULL);

      return 1;
    }
    while ((bytes_read = read(fd, buf, 1000)) > 0) {
      write(1, buf, bytes_read);
    }
    free(buf);
    close(fd);

    return 1;
  }

  return 1;
}

// Exit builtin command.
int shell_exit(char **args) {
  return 0;
}

// True builtin command.
int shell_true(char **args) {
  return 0;
}

// False builtin command.
int shell_false(char **args) {
  return 1;
}

// Jobs builtin command.
int shell_jobs(char **args) {
  char *jobs_dir = home_dir("jobs.txt");
  FILE *fr = fopen(jobs_dir, "r");
  char line[100];
  int line_count = 1;
  int bufsize = 64;
  char *running_jobs[100];;
  int index = 0;

  while (fgets(line, 100, fr) != NULL && strlen(line) > 0) {
    char *pid = get_pid(line);
    char *cmd = sub_str(line, strlen(pid) + 1, strlen(line) - 1);
    int is_alive = waitpid(atoi(pid), NULL, WNOHANG);

    if (is_alive == 0) {
      printf("[%d] +Running\t%s", line_count, cmd);
      running_jobs[index] = sub_str(line, 0, strlen(line) - 1);
      index++;
    }
    else {
      printf("[%d] -Done\t%s", line_count, cmd);
    }
    line_count++;
  }
  running_jobs[index] = NULL;
  fclose(fr);
  FILE *fw = fopen(jobs_dir, "w");
  for (int i = 0; running_jobs[i] != NULL; i++) 
    fprintf(fw, "%s", running_jobs[i]);
  fclose(fw);

  return 1;
}

// Foreground builtin command.
int shell_fg(char **args) {
  FILE *f = fopen(home_dir("jobs.txt"), "r");
  char line[100];
  int pid, w;

  if (args[1] == NULL) {
    char *n_line;
    while (fgets(line, 100, f) != NULL && strlen(line) > 0) 
      n_line = sub_str(line, 0, strlen(line) - 1);
    pid = atoi(get_pid(n_line));    
  }
  else {
    int bool = 0;
    pid = atoi(args[1]);
    while (fgets(line, 100, f) != NULL && strlen(line) > 0) {
      if (pid == atoi(get_pid(line)))
        bool = 1;
      if (bool != 1) {
        printf("The process is not in the background\n");
        return 1;
      }
    }
  }

  if (waitpid(pid, NULL, WNOHANG) == 0) {
      do {
        w = waitpid(pid, NULL, WNOHANG);
      } while (w == 0);
  }
  else printf("Process finished in background\n");
  
  fclose(f);

  return 1;
}

// History builtin command
int shell_history(char **args) {
  char *history_dir = home_dir("history.txt");
  FILE *fr = fopen(history_dir, "r");
  char line[100];
  int line_count = 1;

  while (fgets(line, 100, fr) != NULL && strlen(line) > 0) {
    printf("%d: %s", line_count, line);
    line_count++;
  }
  fclose(fr);

  return 1;
}

// Again builtin command
char **shell_again(char **args) {
  char *history_dir = home_dir("history.txt");
  FILE *fr = fopen(history_dir, "r");
  char line[100];
  int line_count = 1;
  char **new_args = malloc(100 * sizeof(char *));
  int index = 0;

  while (fgets(line, 100, fr) != NULL && strlen(line) > 0) {
    if (atoi(args[1]) == line_count) {
      char *cmd = sub_str(line, 0, strlen(line) - 2);
      char *token = strtok(cmd, " ");
      while (token != NULL) {
        new_args[index] = token;
        token = strtok(NULL, " ");
        index++;
      }
      new_args[index] = NULL;
      fclose(fr);
      return new_args;
    }
    line_count++;
  }
  fclose(fr);

  return NULL;
}

// Unset bultin command
int shell_unset(char **args) {
  if (args[1] == NULL) {
    printf("unset: not enough arguments\n");
    return 1;
  }
  else {
    char *var = args[1];
    delete_var(var);
  }

  return 1;
}

// Get bultin command
int shell_get(char **args) {
  if (args[1] == NULL) 
    return 1;
  else {
    char *var = args[1];
    char *var_dir = home_dir("variables.txt");
    FILE *f = fopen(var_dir, "r");
    char line[128];

    while (fgets(line, 128, f) != NULL) {
      int index = 0;
      while (line[index] != '\n' && line[index] != '=') index++;
      if (line[index] == '=') {
        char *v = sub_str(line, 0, index - 2);
        if (strcmp(v, var) == 0) {
          printf("%s", sub_str(line, index + 2, strlen(line) - 1));
          while(1) {
            fgets(line, 128, f);
            if (line[0] == '*') break;
            printf("%s", line);
          }

          return 1;
        }
      }
    }
  }

  return 1;
}