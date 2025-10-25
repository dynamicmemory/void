#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../include/document.h"
#include "../include/terminal.h"

#define CTRL_QUIT 17
#define CTRL_SAVE 19
#define ENTER 13
#define BACKSPACE 127
#define CAPACITY 256

// Probably move to own file once input and display are fully written
typedef struct editor {
    int row;
    int col;
} editor;

void init_editor(editor *e) {
    e->row = 1;
    e->col = 1;
}

// Move to display.c file when finished
void render_display(document *d, editor *e);

// Move to input.c file when finished
void input_char(document *d, editor *e, char c);
void backspace(document *d, editor *e);
void enter(document *d, editor *e);
void move_cursor(document *d, editor *e);

int main(int argc, char *argv[]) {
    // Set the terminal into raw mode 
    enableRaw();

    if (argc > 2) {
        perror("Usage: Only enter one filename");
        return 1;
    }
    document d;
    if (init_doc(&d, argv) == -1) {
        fprintf(stderr, "Failed to correctly handle file");
        return -1;
    }

    editor e;
    init_editor(&e);
    render_display(&d, &e);

    char c;
    while (read(0, &c, 1) == 1) {
        if (c == CTRL_QUIT) exit(0);
        if (c == CTRL_SAVE) save_doc(&d);  
        if (c >= 32 && c <= 126) input_char(&d, &e, c);
        if (c == BACKSPACE) backspace(&d, &e);
        if (c == ENTER) enter(&d, &e);
        if (c == '\x1b') move_cursor(&d, &e);
        render_display(&d, &e);
    }

    // --- TODO ---
    // Read in a file  DONE  
    // Store it in a buffer  DONE
    // Print it to the screen  DONE 
    // Breakout of standard terminal mode DONE
    // Move cursor around screen DONE
    // add chars to lines DONE
    // backspace chars 
    // Enter new lines 
    // Viewport adjustment
    // status bar
    // Finally new features
    return 0;
}

// TODO: ERROR handling alloc and memmove
/* Inserts the character typed on the keyboard into the document at the cursors 
 * current location */
void input_char(document *d, editor *e, char c) {
    int row = e->row-1;
    int col = e->col-1;
    char *line = d->lines[row];
    size_t len = strlen(line);

    if (col > len) col = len;

    line = realloc(line, len+2);
    d->lines[row] = line;

    memmove(line + col+1, line + col, len - col + 1);

    line[col] = c; 
    e->col++;

}

/* Logic for when the backspace key is pressed on the keyboard*/
void backspace(document *d, editor *e) {
    int row = e->row-1;
    int col = e->col-1;

    if (e->row > 1 && e->col == 1) {
        e->row = e->row-1;
        e->col = strlen(d->lines[e->row-1]);
        row--;
        col--;

        // char *line = d.lines[row+1];
        // size_t len = strlen(line);
        // line = realloc(line, len - 1);
        // d.lines[row] = line;
        // memmove(line + col-1, line + col, len - 1 - col);
    }
    else if (e->col > 1) {
        // char *line = d.lines[row];
        // size_t len = strlen(line);
        // line = realloc(line, len - 1);
        // d.lines[row] = line;
        // memmove(line + col-1, line + col, len - 1 - col);
        e->col = e->col-1;
    }
}

/* Logic for what happens when a newline is entered on the keyboard*/
void enter(document *d, editor *e) {
    if (d->nlines != e->row) {
        e->row++;
        e->col = 1;
    }
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
            case'A': if (e->row > 1) e->row = e->row - 1; break;
            case'B': if (e->row < d->nlines) e->row = e->row + 1; break;

            // If the current column num is less then the len of the string 
            // in the current row, then we can move the cursor to the right
            case'C': if (e->col < strlen(d->lines[e->row-1])) e->col = e->col + 1; break;
            case'D': if (e->col > 1) e->col = e->col - 1; break;
        }
    }
}

// The current bug has to do with doubling capacity, when we realloc, something 
// breaks, same with version 1, the problem just doesnt appear there because I 
// initialize that buffer with a huge number of bytes, where here its just 256
// TODO: ADD ERROR CHECKING AND HANDLING TO THIS
void render_display(document *d, editor *e) {
    long capacity = 256;
    char *buffer= malloc(capacity);

    // Chars for hiding the cursor, moving it to the top and clearing the screen
    char *escapesec = "\x1b[?25l\x1b[H\x1b[2J";
    int esc_len = strlen(escapesec);
    memcpy(buffer, escapesec, esc_len);

    size_t size = esc_len;
    for (int i = 0; i < d->nlines; i++) {
        char *p = d->lines[i]; 
        while (*p) {
            // Increase buffer size if it runs out
            if (size + 3 >= capacity) {
                capacity *= 2;
                buffer = realloc(buffer, capacity);
            }

            // Write each line to the buffer
            buffer[size] = *p;
            size++;
            p++;
        }
        buffer[size] = '\r';
        size++;
        buffer[size] = '\n';
        size++;
    }

    // Recalculates the col num if the row has changed and the len of 
    // the string in the new row is less then the current col num
    if (strlen(d->lines[e->row-1]) < e->col-1)
        e->col = strlen(d->lines[e->row-1]);

    // Make a string for moving the cursor and unhiding it and add it to the end 
    int n = snprintf(buffer + size, capacity - size, "\x1b[%d;%dH\x1b[?25h", e->row, e->col);
    // If n is less then cap - size, then there wasn't enough space to write full string
    if (n >= capacity - size) {
        // Double cap size and rewrite string again
        capacity *= 2;
        buffer = realloc(buffer, capacity);
        n = snprintf(buffer + size, capacity - size, "\x1b[%d;%dH\x1b[?25h", e->row, e->col);
    }
    // Assign n to the size of the buffer and away we go
    size += n;

    write(STDOUT_FILENO, buffer, size);
}
