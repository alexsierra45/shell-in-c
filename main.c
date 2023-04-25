#include "loop_functions.c"

void init() {
    // Empty the file background.txt
    FILE *fj = fopen(home_dir("jobs.txt"), "w");
    fclose(fj);
    // Check for the existence of the file history.txt
    FILE *fh = fopen(home_dir("history.txt"), "r");
    if (fh == NULL) {
        FILE *fh = fopen(home_dir("history.txt"), "w");
        fclose(fh);
    }
    else 
        fclose(fh);
}

int main(int arcg, char *argv[]) {

    init();
    loop();

    return 0;
}