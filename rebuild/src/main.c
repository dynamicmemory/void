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

void render_display(document *d);

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
    
    render_display(&d);

    int c;
    while ((c=getc(stdin)) != EOF) {
        if (c == CTRL_QUIT) exit(0);
        if (c == CTRL_SAVE) exit(0);  // TODO: Change to save shortly
       
        
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
void render_display(document *d) {
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

    // Chars for unhiding the cursor
    char *showc = "\x1b[?25h";
    int showlen = strlen(showc);
    memcpy(buffer + size, showc, showlen);
    size += showlen;

    write(STDOUT_FILENO, buffer, size);
}
