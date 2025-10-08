#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../include/document.h"

void render_display(document *d);

int main(int argc, char *argv[]) {
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
    // for (int i = 0; i < d.nlines; i++) {
    //     printf("%s\n", d.lines[i]);
    // }

    // save_doc(&d);

    // Read in a file  DONE  
    // Store it in a buffer  DONE
    // Print it to the screen  DONE 
    // Breakout of standard terminal move 
    // Move cursor around screen 
    // add chars to lines 
    // backspace chars 
    // Enter new lines 
    // Edge case
    return 0;
}

void render_display(document *d) {
    long capacity = 256;
    char *buffer= malloc(capacity);

    char *escapesec = "\x1b[?25l\x1b[H\x1b[2J";
    int esc_len = strlen(escapesec);
    memcpy(buffer, escapesec, esc_len);

    size_t size = esc_len;
    for (int i = 0; i < d->nlines; i++) {
        char *p = d->lines[i]; 
        while (*p) {
            if (size == capacity) {
                capacity *= 2;
                buffer = realloc(buffer, capacity);
            }

            buffer[size] = *p;
            size++;
            p++;
        }
        buffer[size] = '\n';
        size++;
    }
    write(STDOUT_FILENO, buffer, size);
}
