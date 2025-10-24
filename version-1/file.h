#ifndef FILE_H 
#define FILE_H 

#include <stdio.h>
#include "session.h"

#define CTRL_Q 17 
#define CTRL_S 19 

char *create_default_file();
char *file_handler(char **arg, FILE **fd);
void save_file(char **lines, char *fname, int nlines);
void read_file(FILE *fd, int *nlines, char ***file);

#endif
