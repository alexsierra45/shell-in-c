#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

// Function Declarations for builtin shell commands:
int shell_cd(char **args);
int shell_help(char **args);
int shell_exit(char **args);
int shell_true(char **args);
int shell_false(char **args);

// List of builtin commands, followed by their corresponding functions.
char *builtin_str[] = {
  "cd",
  "help",
  "exit",
  "true",
  "false"
};

int (*builtin_func[]) (char **) = {
  &shell_cd,
  &shell_help,
  &shell_exit, 
  &shell_true, 
  &shell_false
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
  if (args[1] == NULL) {
    int fd = open("help/general.txt", O_RDONLY);
    char *buf = malloc(1000);
    ssize_t bytes_read;
    while ((bytes_read = read(fd, buf, 1000)) > 0) {
        write(1, buf, bytes_read);
    }
    free(buf);
    close(fd);
  }
  // int i;
  // printf("Shell-in-c's LSH\n");
  // printf("Type program names and arguments, and hit enter.\n");
  // printf("The following are built in:\n");

  // for (i = 0; i < lsh_num_builtins(); i++) {
  //   printf("  %s\n", builtin_str[i]);
  // }

  // printf("Use the man command for information on other programs.\n");
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