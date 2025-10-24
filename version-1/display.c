#include "sysops.h"
#include "session.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define RENDERBUFF 65536

void update_viewport(session *s, int screen_rows, int screen_cols) {
    // setting the vertical offset for the viewport
    if (s->row < s->row_offset) 
        s->row_offset = s->row;
    else if (s->row >= s->row_offset + screen_rows - 1) 
        s->row_offset = s->row - (screen_rows - 1);

    // setting the horizontal offset for the viewport
    if (s->col < s->col_offset) 
        s->col_offset = s->col;
    if (s->col >= s->col_offset + screen_cols)
        s->col_offset = s->col - screen_cols + 1;
}

void build_statusbar(session *s, char *outbuff, size_t *off, int screen_cols) {
    // Invert the colours 
    *off += snprintf(outbuff + *off, RENDERBUFF - *off, "\x1b[1;30;46m");

    char left[128];
    char center[128];
    char right[128];

    // Build the left part of the status bar 
    snprintf(left, sizeof(left), " %s %s ", s->fname, s->dirty ? " *" : "  ");
    // Build the center part of the status bar 
    snprintf(center, sizeof(center), " --VOID-- ");
    // Build the right par of the status bar
    snprintf(right, sizeof(right), " Line %d, Col %d ", s->row + 1, s->col + 1);

    // Get all status bar pieces lengths 
    int left_len = strlen(left);
    int center_len = strlen(center);
    int right_len = strlen(right);

    // Calculate the distances for padding lenghts
    int remaining = screen_cols - (left_len + center_len + right_len);
    int left_pad = remaining / 2;
    int right_pad = remaining - left_pad;

    // Print and pad out the status bar
    *off += snprintf(outbuff + *off, RENDERBUFF - *off, "%s", left);
    for (int i = 0; i < left_pad; i++) 
         *off += snprintf(outbuff + *off, RENDERBUFF - *off, " ");
    *off += snprintf(outbuff + *off, RENDERBUFF - *off, "%s", center);
    for (int i = 0; i < right_pad; i++)
        *off += snprintf(outbuff + *off, RENDERBUFF - *off, " ");
    *off += snprintf(outbuff + *off, RENDERBUFF - *off, "%s", right);

    // Uninvert terminal colours.
    *off += snprintf(outbuff + *off, RENDERBUFF - *off, "\x1b[0m");
}

// Main screen render function
void print_to_screen(session *s) {
    // Get the current screen size
    int screen_rows, screen_cols;
    get_window_size(&screen_rows, &screen_cols);
    screen_rows = screen_rows - 1;

    update_viewport(s, screen_rows, screen_cols);

    // Buffer to hold the output before writing to the screen
    char outbuff[RENDERBUFF];
    size_t off = 0;

    off += snprintf(outbuff + off, RENDERBUFF - off, "\x1b[?25l\x1b[H\x1b[2J");

    // Printing the file to the screen
    for (int i = 0; i < screen_rows; i++) {
        int file_row = i + s->row_offset;
        if (file_row >= s->nlines) 
            off += snprintf(outbuff + off, RENDERBUFF - off, "~\r\n");

        else  {
            // This stops lines from spitting out random chars as they are reading 
            // unassigned memory when one line goes off screen. A Y clamp essentially
            int len = strlen(s->file[file_row]);
            int max = len - s->col_offset;
            if (max < 0) 
                max = 0;
            int print_cols = max < screen_cols ? max : screen_cols;
            
            off += snprintf(outbuff + off, RENDERBUFF - off, "%.*s\r\n", 
                            print_cols, s->file[file_row] + s->col_offset);
        }
    }

    build_statusbar(s, outbuff, &off, screen_cols); 
    
    // Move the cursor into position and then unhide it
    off += snprintf(outbuff + off, RENDERBUFF - off, 
                    "\x1b[%d;%dH\x1b[?25h",
                    s->row - s->row_offset + 1, 
                    s->col - s->col_offset + 1);

    // Write the buffer to the terminal window, wooow no flicker now 
    write(STDOUT_FILENO, outbuff, off);
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
