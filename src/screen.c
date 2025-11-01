#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../include/document.h"
#include "../include/input.h"

#define CURSOR_HIDE_CLEAR "\x1b[?25l\x1b[H\x1b[2J"
#define CURSOR_BOTTOM_ROW "\x1b[%d;%dH"
#define STATUS_BAR_COLOUR "\x1b[1;30;46m"
#define CURSOR_RESTORE "\x1b[%d;%dH\x1b[?25h" 
#define RESTORE_COLOUR "\x1b[0m"
#define MIN(a,b) ((a)<(b)?(a):(b))

typedef struct {
    size_t cap;
    size_t size;
    char *buff; 
} screen;

// Move to display.c file when finished
void init_screen(screen *s);
void render_screen(document *d, editor *e, screen *s);
void update_viewport(document *d, editor *e);
void adjust_buff_size(screen *s, int limit);
void write_to_buff(screen *s, char *data, int len);
void render_status_bar(document *d, editor *e, screen *s);
void reset_screen(screen *s);

/* initializes the screen rendering structure */
void init_screen(screen *s) {
    s->cap = 256;
    s->size = 0;
    s->buff = malloc(s->cap);
}

/* Main function in charge of building and displaying the text file to the 
 * terminal window */
void render_screen(document *d, editor *e, screen *s) { 
    s->size = strlen(CURSOR_HIDE_CLEAR); 
    memcpy(s->buff, CURSOR_HIDE_CLEAR, s->size);
    update_viewport(d, e); 
    
    // Screen rows -1 to make room for the status bar 
    for (int i = 0; i < e->screen_rows-1; i++) { 
        if (i + e->row_offset >= d->nlines) 
            write_to_buff(s, "~\r\n", 3); 
        else { 
            char *p = d->lines[i + e->row_offset]; // show lines within range 
            // clamping for cols, either str length or offset depending on size 
            int stop = MIN(strlen(p), e->col_offset + e->screen_cols); 
            for (int j = e->col_offset; j < stop; j++) 
                write_to_buff(s, &p[j], 1); 
            // Dont print nl if its the last line of screen (-1 for status bar) 
            write_to_buff(s, "\r", 1); 
            if (i < e->screen_rows - 1) write_to_buff(s, "\n", 1); 
        } 
    } 
    // ensures col snaps to new rows length if smaller then previous row 
    e->col = MIN(strlen(d->lines[e->row]), e->col); 
    render_status_bar(d, e, s); 

    adjust_buff_size(s, 64); 
    int c_col = e->col+1 - e->col_offset; 
    int c_row = e->row+1 - e->row_offset; 
    c_row = c_row > e->screen_rows-1 ? e->screen_rows-1 : c_row; 
    s->size += snprintf(s->buff+s->size, s->cap-s->size, CURSOR_RESTORE, c_row, c_col); 

    write(STDOUT_FILENO, s->buff, s->size); 
    reset_screen(s); 
}   

/* Calculates which rows/cols of the file should be shown in the terminal 
 * window, depending on where the user currently is in the file */
void update_viewport(document *d, editor *e) {
    int screen_rows = e->screen_rows-1;
    if (e->row < e->row_offset) 
        e->row_offset = e->row;
    else if (e->row >= e->row_offset + screen_rows) 
        e->row_offset = e->row - screen_rows+1;

    // col offset calcs
    if (e->col + e->col_offset < e->screen_cols) 
        e->col_offset = 0;
    else if (e->col + e->col_offset >= e->screen_cols)
        e->col_offset = e->col - e->screen_cols;
}

/* Checks how close the screen buffer is to being at max capacity, automatically
 * doubles it's size if it's within a specified limit.*/
void adjust_buff_size(screen *s, int limit) {
    if (s->size + limit >= s->cap) {
        s->cap *= 2;
        char *tmp = realloc(s->buff, sizeof(char) * s->cap);

        if (!tmp) exit(1);
        s->buff = tmp;
    }
}

/* Helper function for display_screen, Writes the passed in data to the screens 
 * buffer */
void write_to_buff(screen *s, char *data, int len) {
    adjust_buff_size(s, len);
    memcpy(s->buff + s->size, data, len);
    s->size += len;
}

/* Handles the building and rendering of the status bar on the bottom row of 
 * the text editor */
void render_status_bar(document *d, editor *e, screen *s) {
    adjust_buff_size(s, 512);

    s->size += snprintf(s->buff+s->size, s->cap-s->size, CURSOR_BOTTOM_ROW, e->screen_rows, 0);
    s->size += snprintf(s->buff+s->size, s->cap-s->size, STATUS_BAR_COLOUR);
    
    char left[256];
    char center[256];
    char right[256];

    int l_len = snprintf(left, sizeof(left), "%s %s",d->dname, d->dirty > 0 ? "*" : " ");
    int c_len= snprintf(center, sizeof(center), " --VOID-- ");
    int r_len = snprintf(right, sizeof(right), "line %d, col %d",e->row, e->col);
   
    int remaining = e->screen_cols - (l_len + c_len + r_len);
    int l_pad = remaining / 2;
    int r_pad = remaining - l_pad;

    s->size += snprintf(s->buff+s->size, s->cap-s->size, "%s", left);
    for (int i = 0; i < l_pad; i++)
        s->size += snprintf(s->buff+s->size, s->cap-s->size, " ");
    s->size += snprintf(s->buff+s->size, s->cap-s->size, "%s", center);
    for (int i = 0; i < r_pad; i++)
        s->size += snprintf(s->buff+s->size, s->cap-s->size, " ");
    s->size += snprintf(s->buff+s->size, s->cap-s->size, "%s", right);
    
    s->size += snprintf(s->buff+s->size, s->cap-s->size, RESTORE_COLOUR);
}

/* Resets the size of the screen back to 0 for the next render cycle */
void reset_screen(screen *s) {
    s->size = 0;
}
