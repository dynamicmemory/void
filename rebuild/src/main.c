#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../include/file_ops.h"
#include "../include/file_manager.h"

int init_file(char **args, file *e);
int read_file(file *e);
int save_file(file *e);



int main(int argc, char *argv[]) {
    if (argc > 2) {
        perror("Usage: Only enter one filename");
        return 1;
    }
    file e;
    if (init_editor(&e, argv) == -1) {
        fprintf(stderr, "Failed to correctly handle file");
        return -1;
    }
    
    for (int i = 0; i < e.nlines; i++) {
        printf("%s", e.lines[i]);
    }
    save_file(&e);
    // Read in a file  DONE  
    // Store it in a buffer  DONE
    // Print it to the screen  DONE 
    // Breakout of standard terminal move 
    // Move cursor around screen 
    // add chars to lines 
    // backspace chars 
    // Enter new lines 
    // Edge case
    return 0;
}

/*
 * Sets the file name for the editor struct from the file name passed in, if one 
 * was not provided, it will create a file, if the file name passed in doesn't 
 * exist, it will create a file with that name 
 * Params: char ** command line arguments
 * Returns: char * filename
 */
int init_file(char **args, file *e) {
    char fname_buff[256]; 
    char *fname = fname_buff;

    // If no filename was provided, make a new file for the user
    if (args[1] == NULL) {
        int counter = 0;
        while (1) {
            // Craft the filename
            snprintf(fname_buff, 256, "newvoid%d.txt", ++counter);
            // Test if the filename exists, if not, break out of the loop
            FILE *fd = fopen(fname_buff, "r");
            if (fd == NULL) break;
            // close the file if we were successfully able to open it.
            fclose(fd);
        }
        // Create the file and return the name
        FILE *fd = fopen(fname_buff, "w");
        // fwrite("", sizeof(char), 1, fd);            
        fclose(fd);
    }
    else {
        // Try open provided filename, if it doesnt exist, create it, check again
        FILE *fd = fopen(args[1], "r");
        if (fd == NULL) fd = fopen(args[1], "w");
            // fwrite("", sizeof(char), 1, fd);                
        // Exit if unable to create it as well
        if (fd == NULL) {
            printf("Unable to open file: %s\n", args[1]);
            exit(1);
        }
        // close the file and set the fname to the provided filename.
        fclose(fd);
        fname = args[1];    
    }
    e->fname = strdup(fname);
    if (e->fname == NULL) return -1;
    return 0;
}

/* Reads a file into the editors char **array for editing and operations 
 * Params: struct editor e
 * Returns: 0 if successful and -1 if an error occured.
 */
int read_file(file *e) {
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

/* Saves the current file.
 * Params: e - editior struct
 * Returns: 0 if successful and -1 if an error occured.
 */
int save_file(file *e) {
    FILE *fd = fopen(e->fname, "w");
    if (fd == NULL) return -1;

    for (int i = 0; i < e->nlines; i++) {
        fputs(e->lines[i], fd);
        fputc('\n', fd);
        if (ferror(fd)) {
            fclose(fd);
            return -1;
        }
    }

    fclose(fd);
    return 0;
}
