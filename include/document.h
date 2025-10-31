#ifndef DOCUMENT_H  
#define DOCUMENT_H

/* 
 * document.h
 *
 * Defines the document managment for the text editor:
 * - init_doc: initializes the file passed in for editing or creates a files. 
 * - init_name: initializes the files name passed in for editing or creates a files. 
 * - read_doc: Reads the contents of the file into the file buffer.
 * - save_doc: Saves the contents of the file buffer back into the file.
 */ 

typedef struct document {
    char **lines;
    char *dname;
    int nlines;
    int capacity;
    int dirty;
} document;

int init_doc(document *d, char **args);
int init_name(document *d, char **args);
int read_doc(document *d);
int save_doc(document *d);

#endif 

