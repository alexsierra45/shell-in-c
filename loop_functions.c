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

void loop();

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
  return tokens;
}

void manage_ctrl_c(int signal) {
    printf("\n");
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
    status = execute(args, 0, 1);
    free(line);
    free(args);
  }
}