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

// True builtin command.
int shell_true(char **args) {
  return 1;
}

// False builtin command.
int shell_false(char **args) {
  return 0;
}