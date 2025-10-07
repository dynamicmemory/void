#ifndef FILE_OPS_H
#define FILE_OPS_H 

/* 
 * file_ops.h
 *
 * Defines the file managment for the text editor:
 * - init_file: initializes the file passed in for editing or creates a files. 
 * - read_file: Reads the contents of the file into the file buffer.
 * - save_file: Saves the contents of the file buffer back into the file.
 */ 

#include "file_manager.h"

int init_file(char **args, file *f);
int read_file(file *f);
int save_file(file *f);

#endif 

