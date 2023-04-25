#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <pwd.h>
#include "execute.c"

#define BLUE "\033[1;34m"
#define RESET "\033[0m"
#define GREEN "\033[1;32m"

int count = 0;

char *read_line(void) {
  char *line = NULL;
  ssize_t bufsize = 0; // have getline allocate a buffer for us

  if (getline(&line, &bufsize, stdin) == -1){
    if (feof(stdin)) {
      exit(EXIT_SUCCESS);  // We recieved an EOF
    } else  {
      perror("readline");
      exit(EXIT_FAILURE);
    }
  }

  return line;
}

char *decod_line(char *line) {
    char *aux_line = (char *) malloc(3 * strlen(line));

    int len = (int) strlen(line);
    int j = 0;
    for (int i = 0; i < len; i++) {
        if (line[i] == '#')
            break;

        if (line[i] == ' ') {
            if (i != 0) {
                if (line[i - 1] == ' ') continue;
            } else
                continue;
        }

        if (line[i] == ';') {
            if (i != 0) {
                if (line[i - 1] != ' ') {
                    aux_line[j++] = ' ';
                }
            }
            aux_line[j++] = line[i];
            if (i != len - 1) {
                if (line[i + 1] != ' ') {
                    aux_line[j++] = ' ';
                }
            }

            continue;
        }

        if (line[i] == '<') {
            if (i != 0) {
                if (line[i - 1] != ' ') {
                    aux_line[j++] = ' ';
                }
            }
            aux_line[j++] = line[i];
            if (i != len - 1) {
                if (line[i + 1] != ' ') {
                    aux_line[j++] = ' ';
                }
            }

            continue;
        }

        if (line[i] == '>') {
            if (i != 0) {
                if (line[i - 1] != ' ' && line[i - 1] != '>') {
                    aux_line[j++] = ' ';
                }
            }
            aux_line[j++] = line[i];
            if (i != sizeof(line) - 1) {
                if (line[i + 1] != ' ' && line[i + 1] != '>') {
                    aux_line[j++] = ' ';
                }
            }

            continue;
        }

        if (line[i] == '|') {
            if (i != 0) {
                if (line[i - 1] != ' ' && line[i - 1] != '|') {
                    aux_line[j++] = ' ';
                }
            }
            aux_line[j++] = line[i];
            if (i != len - 1) {
                if (line[i + 1] != ' ' && line[i + 1] != '|') {
                    aux_line[j++] = ' ';
                }
            }

            continue;
        }

        if (line[i] == '&') {
            if (i != len - 1 && i != 0) {
                if (line[i - 1] != ' ' && line[i + 1] == '&') {
                    aux_line[j++] = ' ';
                }
            }
            aux_line[j++] = line[i];
            if (i != 0 && i != len - 1) {
                if (line[i + 1] != ' ' && line[i - 1] == '&') {
                    aux_line[j++] = ' ';
                }
            }

            continue;
        }

        aux_line[j++] = line[i];
    }

    char *new_line = (char *) malloc(j + 1);

    for (int x = 0; x < j; x++) {
        new_line[x] = aux_line[x];
    }
    free(aux_line);

    if (new_line[j - 1] != '\n') new_line[j++] = '\n';
    new_line[j] = 0;

    return (char *) new_line;
}

#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"
char **split_line(char *line) {
  char *decoded_line = decod_line(line);
  int bufsize = LSH_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token;

  if (!tokens) {
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(decoded_line, LSH_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += LSH_TOK_BUFSIZE;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, LSH_TOK_DELIM);
  }
  tokens[position] = NULL;
  if (strcmp(tokens[0], "again") == 0) {
    char *args[3] = {tokens[0], tokens[1], NULL};
    tokens = shell_again(args);
  }
  return tokens;
}

void manage_ctrl_c(int signal) {
    printf("\n");
    if (count) exit(signal);
    count++;
}

void write_history(char **args) {
    char *history_dir = home_dir("history.txt");
    int c_lines = count_lines(history_dir);
    char *command = concat_array(args, ' ');
    FILE *f = fopen(history_dir, "r");
    char *lines[9];

    if (c_lines == 10) {
        char line[128];
        fgets(line, 128, f);
        for (int i = 0; i < 9; i++) {
            fgets(line, 128, f);
            lines[i] = sub_str(line, 0, strlen(line) - 1);
        } 
        f = fopen(history_dir, "w");
        for (int i = 0; i < 9; i++) 
            fprintf(f, "%s", lines[i]);
        fclose(f);
    } 
    f = fopen(history_dir, "a");
    fprintf(f, "%s\n", command);
    fclose(f);
}

void loop() {
  char *line;
  char **args;
  int status = 1;
  char cwd[128];
  char *prompt = "\033[0;32mmyshell\033[0m";
  struct passwd *pw = getpwuid(getuid());

  signal(SIGINT, manage_ctrl_c);
  while (status) {
    getcwd(cwd, sizeof(cwd));
    printf("%smyshell@%s%s:%s~%s%s$ ", GREEN, pw->pw_name, RESET, BLUE, cwd, RESET);
    line = read_line();
    args = split_line(line);
    status = execute(args, 0, 1, 1);
    if (status != -1)
        write_history(args);
    free(line);
    free(args);
  }
}