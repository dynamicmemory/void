#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct editor {
    char **lines;
    char *fname;
    int nlines;
    int capacity;
} editor;

char *init_file(char **args);
void add_line(editor *e);
int read_lines(editor *e);

void init_editor(editor *e, char **arg) {
    e->lines = NULL;
    e->fname = init_file(arg);
    e->capacity = 0;
    if (read_lines(e) == -1) fprintf(stderr, "Failed to read file.");

}

int main(int argc, char *argv[]) {
    if (argc > 2) {
        perror("Usage: Only enter one filename");
        return 1;
    }
    editor e;
    init_editor(&e, argv);

    // Read in a file  DONE  
    // Store it in a buffer  DONE
    // Print it to the screen 
    // Breakout of standard terminal move 
    // Move cursor around screen 
    // add chars to lines 
    // backspace chars 
    // Enter new lines 
    // Edge case
    return 0;
}

int read_lines(editor *e) {
    FILE *fd = fopen(e->fname, "r");
    if (fd == NULL) return -1;

    char *line = NULL;
    char *copy = NULL;
    size_t length = 0;
    while (getline(&line, &length, fd) != -1) {
        // duplicate the string so that everyline has its own space in memory
        copy = strdup(line);
        if (copy == NULL) goto cleanup;

        if (e->nlines == e->capacity) {
            // Double capacity if not 0 otherwise add 1 to avoid 0 from multi
            e->capacity = (e->capacity == 0) ? 1 : e->capacity * 2;

            void *temp = realloc(e->lines, e->capacity * sizeof(char*));
            if (temp == NULL) goto cleanup;
            else e->lines = temp;
        }
        // Copy the line into the next line in the array of lines  
        e->lines[e->nlines] = copy;
        e->nlines += 1;
        line = NULL;
    }
    // Free the last line read in to avoid a memory leak if file isnt empty
    if (e->nlines) free(line);
    fclose(fd);
    return 0;

    // Cleanup pointer if an error occurs
    cleanup:
        if (copy != NULL) free(copy);
        for (int i = 0; i < e->nlines; i++) free(e->lines[i]);
        free(e->lines);
        e->lines = NULL;
        e->nlines = e->capacity = 0;
        fclose(fd);
        return -1;
}

/*
Sets the file name for the editor struct from the file name passed in, if one 
was not provided, it will create a file, if the file name passed in doesn't 
exist, it will create a file with that name 
Params: char ** command line arguments
Returns: char * filename
*/
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
            if (fd == NULL) break;
        }
        // Create the file and return the name
        FILE *fd = fopen(fname_buff, "w");
        fclose(fd);
    }
    else {
        // Try open provided filename, if it doesnt exist, create it, check again
        FILE *fd = fopen(args[1], "r");
        if (fd == NULL) fd = fopen(args[1], "w");
                
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

void add_line(editor *e) {
    FILE *fd = fopen(e->fname, "w");
    size_t size = 1;
    fwrite("", sizeof(char), 1, fd);
    e->nlines++;
}
