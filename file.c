#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"

#define CTRL_Q 17 
#define CTRL_S 19 

// If a file isnt provided as an argument, then this function creates a file 
// with our predetermined naming convention.
char *create_default_file() {
    static char fname[256];
    int index = 0;

    // Loops until we find a filename that doesn't exist, returns that name 
    while (1) {
        if (index == 0) 
            snprintf(fname, sizeof(fname), "newvoid.txt");
        else 
            snprintf(fname, sizeof(fname), "newvoid%d.txt", index);

        FILE *fd = fopen(fname, "r");
        if (!fd) 
            break;
        
        fclose(fd);
        index++;
    }
    return fname;
}

// Opens provided file or creates one if needed
char *file_handler(char **arg, FILE **fd) {
    char *fname;
    // If an argument was provided then try open the file 
    if (arg[1]) {
        fname = arg[1];
        *fd = fopen(fname, "r");    
    
        // If the file failed to open, try to create it with the name 
        if (!*fd) *fd = fopen(fname, "w");
        // If the file could not be created exit according
        if (!*fd) {
            perror("Error: Unable to create file");
            exit(1);
        }
    }
    // If no file name was provide, call create a file
    else {
        fname = create_default_file();
        *fd = fopen(fname, "w");

        if (!*fd) {
            perror("Error: Unable to create file");
            exit(1);
        }
    }
    return fname;
}

void save_file(char **lines, char *fname, int nlines) {
    // Open the file, get the fd and format print it 
    FILE *fd = fopen(fname, "w");
    if (fd) {
        for (int i = 0; i < nlines; i++)
            fprintf(fd, "%s\n", lines[i]);
    }
    else 
        perror("Error: failed to open the file for saving");
}

// Reads through the provided fd and saves it all to a 2d array 
void read_file(FILE *fd, int *nlines, char ***file) {
    char *line = NULL; 
    size_t len = 0;
    while (getline(&line, &len, fd) != -1) {
        size_t linelen = strlen(line);

        // Removes the \n from the end of all lines to avoid diagonal printing 
        if (linelen > 0 && line[linelen - 1] == '\n') line[linelen - 1] = '\0'; 

        // Resize and reallocate the array in memory for each line added 
        char **temp = realloc(*file, sizeof(char*) * (*nlines + 1));
        // Check for allocation failure
        if (!temp) { 
            perror("Error: realloc failed"); 
            exit(1); 
        }

        *file = temp;  // Assign reallo to file.

        // Duplicate the line added or else each pointer will overwrite the last 
        (*file)[*nlines] = strdup(line);  // Deref, then get line
        // Check for duplcation failure
        if (!(*file)[*nlines]) { 
            perror("Error: String duplcation failed"); 
            exit(1); 
        }

        (*nlines)++;  // Deref, then increment
    }
    free(line);
    fclose(fd);

    if (*nlines == 0) {
        *file = malloc(sizeof(char*));
        (*file)[0] = strdup("");  // Deref, then get line
        if (!(*file)[0]) { 
            perror("Error: String duplcation failed"); 
            exit(1);
        }

        (*nlines)++;   // defer, then increment
    }
}
