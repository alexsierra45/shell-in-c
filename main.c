#include "loop_functions.c"

void init() {
    // Empty the file background.txt
    FILE *fj = fopen(home_dir("jobs.txt"), "w");
    fclose(fj);
    // Empty the file variables.txt
    FILE *fv = fopen(home_dir("variables.txt"), "w");
    fclose(fv);
    // Check for the existence of the file history.txt
    FILE *fh = fopen(home_dir("history.txt"), "r");
    if (fh == NULL) {
        FILE *fh = fopen(home_dir("history.txt"), "w");
        fclose(fh);
    }
    else 
        fclose(fh);
        
    // Clear the screen
    char *args[2] = {"clear", NULL};
    int pid = fork();
    if (pid == 0) {
        execvp(args[0], args);
        exit(0);
    }
    else wait(NULL);
    printf("Welcome to my shell!\n");
}

int main(int arcg, char *argv[]) {

    init();
    loop();

    return 0;
}