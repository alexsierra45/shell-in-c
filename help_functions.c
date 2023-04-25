#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MY_SH_TOK_DELIM " \t\r\n\a"
#define ERROR "\033[1;31mmy_sh\033[0m"
#define LSH_TOK_BUFSIZE 64

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

// count lines in a file
int count_lines(char *path) {
    FILE *file = fopen(path, "r");
    int lines = 0;
    int c;
    while ((c = fgetc(file)) != EOF) {
        if (c == '\n') {
            lines++;
        }
    }
    fclose(file);
    return lines;
}

// concat the elements of an array
char* concat_array(char** array) {
  int array_size = 0;
  while (array[array_size] != NULL)
    array_size++;
  int total_length = 0;
  for (int i = 0; i < array_size; i++) {
    total_length += strlen(array[i]);
      if (i != array_size - 1) {
        total_length++;
      }
  }

  char* result = (char*) malloc((total_length + 1) * sizeof(char));

  int index = 0;
  for (int i = 0; i < array_size; i++) {
      int len = strlen(array[i]);
      memcpy(result + index, array[i], len);
      index += len;
      if (i != array_size - 1) {
          result[index++] = ' ';
      }
  }

  result[total_length] = '\0';

  return result;
}

// return a substring
char *sub_str(char *line, int init, int end) {
  char *new_line = (char *) malloc(end - init + 1);

  int i;
  for (i = 0; i < end - init + 1; i++) {
      new_line[i] = line[i + init];
  }
  new_line[i] = 0;

  return new_line;
}

// return the pid of a process
char *get_pid(char *line) {
  int i;
  for (i = 0; i < strlen(line); i++) {
    if (line[i] == ' ') break;
  }
  return sub_str(line, 0, i-1);
}