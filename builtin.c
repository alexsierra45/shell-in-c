#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <dirent.h>

// Function Declarations for builtin shell commands:
int shell_cd(char **args);
int shell_help(char **args);
int shell_exit(char **args);
int shell_true(char **args);
int shell_false(char **args);
int shell_jobs(char **args);

char *sub_str(char *line, int init, int end) {
  char *new_line = (char *) malloc(end - init + 1);

  int i;
  for (i = 0; i < end - init + 1; i++) {
      new_line[i] = line[i + init];
  }
  new_line[i] = 0;

  return new_line;
}

char *get_pid(char *line) {
  int i;
  for (i = 0; i < strlen(line); i++) {
    if (line[i] == ' ') break;
  }
  return sub_str(line, 0, i-1);
}

// List of builtin commands, followed by their corresponding functions.
char *builtin_str[] = {
  "cd",
  "help",
  "exit",
  "true",
  "false",
  "jobs",
};

int (*builtin_func[]) (char **) = {
  &shell_cd,
  &shell_help,
  &shell_exit, 
  &shell_true, 
  &shell_false,
  &shell_jobs
};

int lsh_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

// Builtin function implementations.

// Change directory.
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
  FILE *f = fopen("background/jobs.txt", "r");
  char line[100];
  int line_count = 1;

  while (fgets(line, 100, f) != NULL) {
    char *pid = get_pid(line);
    char *cmd = sub_str(line, strlen(pid) + 1, strlen(line) - 1);
    int is_alive = kill(atoi(pid), 0);

    if (is_alive == 0)
      printf("[%d] +Running\t%s", line_count, cmd);
    else {
      printf("[%d] -Done\t%s\n", line_count, cmd);
    }
    line_count++;
  }

  return 1;
}