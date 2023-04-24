#include <stdio.h>
#include "loop_functions.c"

int main(int arcg, char *argv[]) {

    // Empty the file background.txt
    FILE *f = fopen("background/jobs.txt", "w");
    fclose(f);

    loop();

    return 0;
}