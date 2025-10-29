// --- TODO ---
// Read in a file                     DONE  
// Store it in a buffer               DONE
// Print it to the screen             DONE 
// Breakout of standard terminal mode DONE
// Move cursor around screen          DONE
// add chars to lines                 DONE
// backspace chars                    DONE
// Enter new lines                    DONE
// Viewport adjustment
// status bar
// Finally new features
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../include/document.h"
#include "../include/terminal.h"

// For terminal window size 
#include <sys/ioctl.h>

#define CTRL_QUIT 17
#define CTRL_SAVE 19
#define ENTER 13
#define BACKSPACE 127
#define CAPACITY 256

// Probably move to own file once input and display are fully written
typedef struct editor {
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
    ioctl(0, TIOCGWINSZ, &ws);
    e->screen_rows = ws.ws_row;
    e->screen_cols = ws.ws_col;
}

// Move to display.c file when finished
void render_display(document *d, editor *e);
void update_viewport(document *d, editor *e);


// Move to input.c file when finished
void input_handler(document *d, editor *e, char c);
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
        input_handler(&d, &e, c);
        render_display(&d, &e);
    }
    return 0;
}


/* ----------------- INPUT RELATED CODE --------------------- */

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
            case'B': if (e->row < d->nlines-1) e->row++; break;
            // +1 for \0 terminator
            case'C': if (e->col < strlen(d->lines[e->row])+1) e->col++; break;
            case'D': if (e->col > 0) e->col--; break;
        }
    }
}


/* ----------------- DISPLAY/RENDER RELATED CODE --------------------- */

// TODO: Refactor this code into smaller pieces.
// TODO: ADD ERROR CHECKING AND HANDLING TO THIS
void render_display(document *d, editor *e) {
    long capacity = 64;
    char *buffer= malloc(capacity);

    // Chars for hiding the cursor, moving it to the top and clearing the screen
    char *escapesec = "\x1b[?25l\x1b[H\x1b[2J";
    size_t size = strlen(escapesec);
    memcpy(buffer, escapesec, size);

    for (int i = 0; i < d->nlines; i++) {
        char *p = d->lines[i]; 
        while (*p) {
            // Increase buffer size if it runs out
            if (size >= capacity) {
                capacity *= 2;
                char *tmp = realloc(buffer, sizeof(char) * capacity);
                if (!tmp) exit(1);
                buffer = tmp;
            }
            // Write each line to the buffer
            buffer[size++] = *p++;
        }
        // Must check size again otherwise we can stumble over the malloced memory
        if (size >= capacity) {
            capacity *= 2;
            char *tmp = realloc(buffer, sizeof(char) * capacity);
            if (!tmp) exit(1);
            buffer = tmp;
        }
        // Add the return and newline char to the end of the line 
        buffer[size++] = '\r';
        buffer[size++] = '\n';
    }
    // Recalculates the col num if the row has changed and the len of 
    // the string in the new row is less then the current col num
    if (strlen(d->lines[e->row]) < e->col)
        e->col = strlen(d->lines[e->row]);

    // Must check size again otherwise we can stumble over the malloced memory
    if (size >= capacity) {
        capacity *= 2;
        char *tmp = realloc(buffer, sizeof(char) * capacity);
        if (!tmp) exit(1);
        buffer = tmp;
    }
    // Double cap size and rewrite string again
    int n = snprintf(buffer + size, capacity - size, 
                 "\x1b[%d;%dH\x1b[?25h", 
                 e->row+1, e->col+1);

    // Assign n to the size of the buffer and away we go
    size += n;

    write(STDOUT_FILENO, buffer, size);
}

/* Calculates which rows/cols of the file should be shown in the terminal 
 * window, depending on where the user currently is in the file */
void update_viewport(document *d, editor *e) {

}
