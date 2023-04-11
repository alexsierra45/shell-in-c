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