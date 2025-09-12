#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include "session.h"

void get_window_size(int *rows, int *cols) {
    struct winsize ws; 

    if (ioctl(STDOUT_FILENO , TIOCGWINSZ, &ws) == -1) {
        perror("Error: ioctrl - unable to get window");
        *rows = 24;
        *cols = 80;
    }
    else {
        *rows = ws.ws_row;
        *cols = ws.ws_col;
    }
}

void draw_status_bar(session *s) {
    int rows, cols;
    get_window_size(&rows, &cols);

    // Advance to the final line on screen
    printf("\x1b[%d;1H", rows);

    // Invert the colours 
    printf("\x1b[1;30;46m");

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
    int remaining = cols - (left_len + center_len + right_len);
    int left_pad = remaining / 2;
    int right_pad = remaining - left_pad;

    // Print and pad out the status bar
    printf("%s", left);
    for (int i = 0; i < left_pad; i++) 
        putchar(' ');
    printf("%s", center);
    for (int i = 0; i < right_pad; i++)
        putchar(' ');
    printf("%s", right);

    // Uninvert terminal colours.
    printf("\x1b[0m");
}

// void viewport(session *s) {
//     int screen_rows, screen_cols;
//     get_window_size(&screen_rows, &screen_cols);
//
//     if (s->row < screen_rows) 
//         s->row_offset = s->row;
//     else if (s->row >= s->row_offset + screen_rows) 
//         s->row_offset = s->row - screen_rows + 2;
//
//     if (s->col < screen_cols) 
//         s->col_offset = s->col;
//     if (s->col >= s->col_offset + screen_cols)
//         s->col_offset = s->col - screen_cols + 1;
//
//     for (int i = 0; i < screen_rows; i++) {
//         int file_row = i + s->row_offset;
//         if (file_row >= s->nlines) 
//             printf("~\r\n");
//         else 
//             printf("%.*s\r\n", screen_cols, s->file[file_row] + s->col_offset);
//     }
// }
