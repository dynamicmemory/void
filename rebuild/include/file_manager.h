#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H 

#include <stdio.h>

/* 
 * fbuff.h 
 * 
 * Defines the document struct that holds all of the information and operations 
 * related to the document. 
 */ 

typedef struct file {
    char **lines;
    char *fname;
    int nlines;
    int capacity;
} file;

#endif 
