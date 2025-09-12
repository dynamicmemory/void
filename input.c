#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "session.h"
#include "file.h"
#include "statusbar.h"

// Clears the terminal screen completely
void clear_screen() { printf("\x1b[2J"); }

// Moves the cursor to the given row and column
void move_cursor(int row, int col) { printf("\x1b[%d;%dH", row, col); }

// Reprints the screen after every char input
void print_to_screen(session *s) {
    clear_screen(); // Clear the screen 
    move_cursor(0, 0); // Move the cursor to the top left to reprint all lines 

    // viewport(s);
    for (int i = 0; i < s->nlines; i++) {
        printf("%s\r\n", s->file[i]); 
    }
    draw_status_bar(s);

    move_cursor(s->row + 1, s->col + 1); // Adding one to both for 0 index offset
    fflush(stdout);
}

void cursor_handler(session *s) {
    char seq[2];

    // Two chars for an arrow press, not just one
    if (read(STDIN_FILENO, &seq[0], 1) != 1) return;
    if (read(STDIN_FILENO, &seq[1], 1) != 1) return;
    // Add or subtract one for the cursor movement
    if (seq[0] == '[') {
        switch(seq[1]) {
            case 'A': if (s->row > 0) {
                        s->row--;
                        break;
            }
            case 'B': if (s->row < s->nlines -1) {
                        s->row++;
                        break;
            }
            case 'C': s->col++; break;
            case 'D': if (s->col > 0) {
                        s->col--;
                        break;
            }
        }
    }
}

void insert_char(session *s, char c) {
    char *line = s->file[s->row];
    size_t len = strlen(line);

    // Make room for the new char
    s->file[s->row] = realloc(line, len + 2);

    memmove(&s->file[s->row][s->col + 1], 
            &s->file[s->row][s->col], len - s->col + 1);

    s->file[s->row][s->col] = c;
    s->col++;
    s->dirty = 1; // Set dirty bit to true if any input comes in 
}

void backspace(session *s) {
    // If the line has at least one char 
    if (s->col > 0) {
    // Get the current line 
        char *line = s->file[s->row];
        size_t len = strlen(line);

        // Move the char one spot ahead back one
        memmove(&line[s->col - 1], &line[s->col], len - s->col + 1);
            s->col--;    
    }
                
    // If there are no chars but there are multiple lines, remove the line
    if (s->col == 0 && s->row > 0) {
        int prevlen = strlen(s->file[s->row - 1]);
        char *temp = realloc(s->file[s->row - 1], 
                              prevlen + strlen(s->file[s->row]) + 1);

        if (!temp) { 
            perror("Error: realloc failed"); 
            exit(1); 
        }
        s->file[s->row - 1] = temp;

        // Stitch the previous line and current line together
        strcat(s->file[s->row - 1], s->file[s->row]);

        // free the current line from memory as its about to become the prev line
        free(s->file[s->row]);

        memmove(&s->file[s->row], &s->file[s->row + 1], 
                sizeof(char*) * (s->nlines - s->row - 1));
        s->nlines--;
        s->row--;
        s->col = prevlen;
    }
    s->dirty = 1; // Set dirty bit to true if any input comes in 
}

void newline(session *s) {
    // Get the current line and the points that will be split to the left and right
    char *current = s->file[s->row];
    char *left = strndup(current, s->col);
    if (!left) {
        perror("Error: strnup failed");
        exit(1);
    }

    char *right = strdup(current + s->col);
    if (!right) {
        perror("Error: strnup failed");
        exit(1);
    }

    char **temp = realloc(s->file, sizeof(char*) * (s->nlines + 1));
    if (!temp) { 
        perror("Error: realloc failed"); 
        exit(1); 
    }
    s->file = temp;

    // Move the line one exttra line down in memory 
    memmove(&s->file[s->row + 2], &s->file[s->row + 1], 
            sizeof(char*) * (s->nlines - s->row - 1));

    // current line now just equals left
    s->file[s->row] = left;
    // New line equals right 
    s->file[s->row + 1] = right;
    s->nlines++;
    s->row++;
    s->col = 0;
    s->dirty = 1; // Set dirty bit to true if any input comes in 
}

void input_manager(session *s, char c) {
    if (read(STDIN_FILENO, &c, 1) == 1) {
        if (c == CTRL_Q) exit(0);
        if (c == CTRL_S) { save_file(s->file, s->fname, s->nlines); s->dirty = 0;}
        // Handles where the cursor is and the arrow key presses
        if (c == '\x1b') cursor_handler(s);
        // Clamp x axis so we cant keep going ad inifinitum on the x axis 
        if (s->col > strlen(s->file[s->row])) 
            s->col = strlen(s->file[s->row]);
        // Handles inserting any char 
        if (c >= 32 && c <= 126) insert_char(s, c);
        if (c == 127) backspace(s);
        if (c == '\n' || c == '\r') newline(s);
    }
}
