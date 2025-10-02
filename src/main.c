#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct editor {
    char **lines;
    char *fname;
    int nlines;
} editor;

char *init_file(char **args);
void read_lines(editor *e);

void init_editor(editor *e, char **arg) {
    e->lines = NULL;
    e->fname = init_file(arg);
    read_lines(e);
}

int main(int argc, char *argv[]) {
    if (argc > 2) {
        perror("Usage: Only enter one filename");
        return 1;
    }
    editor e;
    init_editor(&e, argv);


    // Read in a file 
    // Store it in a buffer 
    // Print it to the screen 
    // Breakout of standard terminal move 
    // Move cursor around screen 
    // add chars to lines 
    // backspace chars 
    // Enter new lines 
    // Edge case
    return 0;
}

void read_lines(editor *e) {
    FILE *fd = fopen(e->fname, "r");

}

char *init_file(char **args) {
    char fname_buff[18]; 
    char *fname = fname_buff;

    // If no filename was provided, make a new file for the user
    if (args[1] == NULL) {
        int counter = 0;
        while (1) {
            // Craft the filename
            snprintf(fname_buff, 18, "newvoid%d.txt", ++counter);
            // Test if the filename exists, if not, break out of the loop
            FILE *fd = fopen(fname_buff, "r");
            if (fd == NULL) 
                break;
        }
        // Create the file and return the name
        FILE *fd = fopen(fname_buff, "w");
        fclose(fd);
    }
    else {
        // Try open provided filename, if it doesnt exist, create it, check again
        FILE *fd = fopen(args[1], "r");
        if (fd == NULL) {
            fd = fopen(args[1], "w");
        }        
        // Exit if unable to create it as well
        if (fd == NULL) {
            printf("Unable to open file: %s\n", args[1]);
            exit(1);
        }
        // close the file and set the fname to the provided filename.
        fclose(fd);
        fname = args[1];    
    }
    return fname;
}
