#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../include/document.h"
#include "../include/terminal.h"

#define CTRL_QUIT 17
#define CTRL_SAVE 19

typedef struct editor {
    int row;
    int col;
} editor;

void init_editor(editor *e) {
    e->row = 0;
    e->col = 0;
}

void render_display(document *d, editor *e);

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
    
    // render_display(&d, &e);

    int c;
    while ((c=getc(stdin)) != EOF) {
        render_display(&d, &e);
        if (c == CTRL_QUIT) exit(0);
        if (c == CTRL_SAVE) exit(0);  // TODO: Change to save shortly
         
        // Cursor related code 
        if (c == '\x1b') {
            char two_chars[2];
            if (read(0, two_chars, 1) == 0) continue;
            if (read(0, two_chars + 1, 1) == 0) continue;

            if (two_chars[0] == '[') {
                switch(two_chars[1]) {
                case'A': e.row = e.row - 1;
                case'B': e.row = e.row + 1;
                case'C': e.col = e.col + 1;
                case'D': e.col = e.col - 1;
                }
            }
        }
        printf("value of row: %d, col %d\n", e.row, e.col);
        
    }

    // --- TODO ---
    // Read in a file  DONE  
    // Store it in a buffer  DONE
    // Print it to the screen  DONE 
    // Breakout of standard terminal mode DONE
    // Move cursor around screen 
    // add chars to lines 
    // backspace chars 
    // Enter new lines 
    // Edge case
    // status bar
    return 0;
}

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
            if (size == capacity) {
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

    // Make a string for moving the cursor and unhiding it and add it to the end 
    int n = snprintf(buffer + size, capacity - size, "\x1b[%d;%d\x1b[?25h", e->row, e->col);
    // If n is less then cap - size, then there wasn't enough space to write full string
    if (n >= capacity - size) {
        // Double cap size and rewrite string again
        capacity *= 2;
        n = snprintf(buffer + size, capacity - size, "\x1b[%d;%d\x1b[?25h", e->row, e->col);
    }
    // Assign n to the size of the buffer and away we go
    size += n;

    write(STDOUT_FILENO, buffer, size);
}
