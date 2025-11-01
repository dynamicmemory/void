#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../include/document.h"
// #include "../include/input.h"

// For terminal window size 
#include <sys/ioctl.h>

#define CTRL_QUIT 17
#define CTRL_SAVE 19
#define ENTER 13
#define BACKSPACE 127
#define CAPACITY 256

typedef struct {
    int row;
    int col;
    int row_offset;
    int col_offset;
    int screen_rows;
    int screen_cols;
} editor;

struct winsize ws;

void init_editor(editor *e) {
    e->row = 0;
    e->col = 0;
    e->row_offset = 0;
    e->col_offset = 0;
    ioctl(0, TIOCGWINSZ, &ws);
    e->screen_rows = ws.ws_row;
    e->screen_cols = ws.ws_col;
};

// Move to input.c file when finished
void init_editor(editor *e);
void input_handler(document *d, editor *e, char c);
void input_char(document *d, editor *e, char c);
void backspace(document *d, editor *e);
void enter(document *d, editor *e);
void move_cursor(document *d, editor *e);

/* Handles all input from the keyboard and redirects it to its corresponding 
 * function. */
void input_handler(document *d, editor *e, char c) {
        if (c == CTRL_QUIT) exit(0);
        if (c == CTRL_SAVE) save_doc(d);  
        if (c >= 32 && c <= 126) input_char(d, e, c);
        if (c == BACKSPACE) backspace(d, e);
        if (c == ENTER) enter(d, e);
        if (c == '\x1b') move_cursor(d, e);
}

// TODO: ERROR handling alloc and memmove
/* Inserts the character typed on the keyboard into the document at the cursors 
 * current location */
void input_char(document *d, editor *e, char c) {
    int row = e->row;
    int col = e->col;
    char *line = d->lines[row];
    size_t len = strlen(line);

    if (col > len) col = len;

    line = realloc(line, len+2);
    d->lines[row] = line;

    memmove(line + col+1, line + col, len - col + 1);

    line[col] = c; 
    e->col++;
    d->dirty++;
}

// TODO: Make variables instead of calls to everything everytime
/* Logic for when the backspace key is pressed on the keyboard*/
void backspace(document *d, editor *e) {
    if (e->row > 0 && e->col == 0) {
        size_t plen = strlen(d->lines[e->row-1]);

        // TODO: Add null check for realloc
        d->lines[e->row-1] = realloc(d->lines[e->row-1], 
                                     plen+1 + 
                                     strlen(d->lines[e->row]+1));

        // Combined current line and prev line into one
        strcat(d->lines[e->row-1], d->lines[e->row]);
        free(d->lines[e->row]);

        // TODO: Add null check for memmove
        memmove(&d->lines[e->row], 
                &d->lines[e->row+1], 
                sizeof(char*) * (d->nlines - e->row - 1));
        
        d->nlines--; 
        e->row--; 
        e->col = plen;
    }
    else if (e->col > 0) {
        char *line = d->lines[e->row];
        size_t len = strlen(line)+1;
        memmove(line + e->col-1, line + e->col, len-e->col);
        e->col--;
    }
    d->dirty++;
}

// TODO: Write error handling and NULL checks for all memops
/* Logic for what happens when a newline is entered on the keyboard*/
void enter(document *d, editor *e) {
    // Increase size of the lines buffer by 1
    d->lines = realloc(d->lines, sizeof(char*) * (d->nlines+1));

    // Everything at the cursor on to its right is assigned this pointer
    char *right = strdup(&d->lines[e->row][e->col]);

    // Shift all lines below the current one, one more down
    memmove(&d->lines[e->row+2], 
            &d->lines[e->row+1], 
            sizeof(char*) * (d->nlines-1 - e->row));

    // Null terminate the current line at the cursor (small memory misstep but its ok)
    d->lines[e->row][e->col] = '\0';

    // Assign the line below that right pointer we made earlier
    d->lines[e->row+1] = right;

    // increment everything
    d->nlines++;
    e->row++;
    e->col = 0;
    d->dirty++;
}

// TODO: Error handling plus remove the exit() on fail, return error code
/* Logic for moving the cursor in the terminal when any directional arrow 
 * is pressed */
void move_cursor(document *d, editor *e) { 
    char two_chars[2];
    if (read(0, &two_chars[0], 1) != 1) exit(1);
    if (read(0, &two_chars[1], 1) != 1) exit(1);

    if (two_chars[0] == '[') {
        switch(two_chars[1]) {
            // Break is hit regardless everytime to stop bouncing cursor behaviour
            case'A': if (e->row > 0) e->row--; break;
            // -1 on nlines as lines start at 1 not 0 like e->row
            case'B': if (e->row < d->nlines-1) e->row++; 
                     // This lines runs the cursor to the end of the last line 
                     // if you continue to hold the down arrow on the last line.
                     else if (e->col < strlen(d->lines[e->row])+1) e->col++;
                     break;
            // +1 for \0 terminator
            case'C': if (e->col < strlen(d->lines[e->row])+1) e->col++; break;
            case'D': if (e->col > 0) e->col--; break;
        }
    }
}
