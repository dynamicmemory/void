#ifndef INPUT_H 
#define INPUT_H 

#include "../include/document.h"

/* 
 * input.h
 *
 * Defines the input management for the text editor:
 * - init_editor: initializes the the editor struct controlling the rows and cols 
 *   of the editor. 
 * - input_handler: initializes the files name passed in for editing or creates a files. 
 */ 
typedef struct {
    int row;
    int col;
    int row_offset;
    int col_offset;
} editor;

// Move to input.c file when finished
void init_editor(editor *e);
void input_handler(document *d, editor *e, char c);
void input_char(document *d, editor *e, char c);
void backspace(document *d, editor *e);
void enter(document *d, editor *e);
void move_cursor(document *d, editor *e);

#endif
