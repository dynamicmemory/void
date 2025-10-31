#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct document {
    char **lines;
    char *dname;
    int nlines;
    int capacity;
    int dirty;
} document;

int init_name(document *d, char **args);
int read_doc(document *d);
int save_doc(document *d);

int init_doc(document *d, char **arg) {
    d->lines = NULL;
    d->capacity = 0;
    d->nlines = 0;
    d->dirty = 0;
    if (init_name(d, arg) == -1) return -1;
    if (read_doc(d) == -1) return -1;

    return 0;
}

/*
 * Sets the file name for the editor struct from the file name passed in, if one 
 * was not provided, it will create a file, if the file name passed in doesn't 
 * exist, it will create a file with that name 
 * Params: char ** command line arguments
 * Returns: char * filename
 */
int init_name(document *d, char **args) {
    char dname_buff[256]; 
    char *dname = dname_buff;

    // If no filename was provided, make a new file for the user
    if (args[1] == NULL) {
        int counter = 0;
        while (1) {
            // Craft the filename
            snprintf(dname_buff, 256, "newvoid%d.txt", ++counter);
            // Test if the filename exists, if not, break out of the loop
            FILE *fd = fopen(dname_buff, "r");
            if (fd == NULL) break;
            // close the file if we were successfully able to open it.
            fclose(fd);
        }
        // Create the file and return the name
        FILE *fd = fopen(dname_buff, "w");
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
        dname = args[1];    
    }
    d->dname = strdup(dname);
    if (d->dname == NULL) return -1;
    return 0;
}

/* Reads a file into the editors char **array for editing and operations 
 * Params: struct editor e
 * Returns: 0 if successful and -1 if an error occured.
 */
int read_doc(document *d) {
    FILE *fd = fopen(d->dname, "r");
    if (fd == NULL) return -1;

    char *line = NULL;
    char *copy = NULL;
    size_t length = 0;
    while (getline(&line, &length, fd) != -1) {
        // duplicate the string so that everyline has its own space in memory
        copy = strdup(line);
        if (copy == NULL) goto cleanup;
        int len = strlen(copy);
        if (len > 0 && copy[len-1] == '\n') copy[len-1] = '\0';

        if (d->nlines == d->capacity) {
            // Double capacity if not 0 otherwise add 1 to avoid 0 from multi
            d->capacity = (d->capacity == 0) ? 1 : d->capacity * 2;

            void *temp = realloc(d->lines, d->capacity * sizeof(char*));
            if (temp == NULL) goto cleanup;
            else d->lines = temp;
        }
        // Copy the line into the next line in the array of lines  
        d->lines[d->nlines] = copy;
        d->nlines++;
        line = NULL;
    }

    if (d->nlines == 0) {
        d->lines = malloc(sizeof(char *));
        if (!d->lines) return -1;
        d->lines[0] = strdup("");
        if (!d->lines[0]) return -1;
        d->nlines = 1;
        d->capacity = 1;
    }

    // Free the last line read in to avoid a memory leak if file isnt empty
    free(line);
    fclose(fd);
    return 0;

    // Cleanup pointer if an error occurs
    cleanup:
        if (copy != NULL) free(copy);
        for (int i = 0; i < d->nlines; i++) free(d->lines[i]);
        free(d->lines);
        d->lines = NULL;
        d->nlines = d->capacity = 0;
        fclose(fd);
        return -1;
}

/* Saves the current file.
 * Params: e - editior struct
 * Returns: 0 if successful and -1 if an error occured.
 */
int save_doc(document *d) {
    FILE *fd = fopen(d->dname, "w");
    if (fd == NULL) return -1;

    for (int i = 0; i < d->nlines; i++) {
        fputs(d->lines[i], fd);
        fputc('\n', fd);
        if (ferror(fd)) {
            fclose(fd);
            return -1;
        }
    }

    fclose(fd);
    d->dirty = 0;
    return 0;
}
