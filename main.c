#include <stdio.h>
#include "loop_functions.c"

void init() {
    // Empty the file background.txt
    FILE *f = fopen("background/jobs.txt", "w");
    fclose(f);
}

int main(int arcg, char *argv[]) {

    init();
    loop();

    return 0;
}