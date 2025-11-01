#ifndef INPUT_H 
#define INPUT_H 

#include "../include/document.h"

/* 
 * input.h
 *
 * Defines the input management for the text editor:
 * - init_doc: initializes the file passed in for editing or creates a files. 
 * - init_name: initializes the files name passed in for editing or creates a files. 
 * - read_doc: Reads the contents of the file into the file buffer.
 * - save_doc: Saves the contents of the file buffer back into the file.
 */ 
typedef struct {
    int row;
    int col;
    int row_offset;
    int col_offset;
    int screen_rows;
    int screen_cols;
} editor;

// Move to input.c file when finished
void init_editor(editor *e);
void input_handler(document *d, editor *e, char c);
void input_char(document *d, editor *e, char c);
void backspace(document *d, editor *e);
void enter(document *d, editor *e);
void move_cursor(document *d, editor *e);

#endif
