#include "file.h"

typedef struct session {
    char *fname;   // File name 
    char **file;   // 2d array of total lines of the file
    int nlines;    // Number of lines the file has 
    int row;       // Current row the cursor is on
    int col;       // Current column the cursor is on
    int row_offset;
    int col_offset; // Viewport offsets
    int dirty;     // Saved condition of the file 0 = saved, 1 = not saved.
} session;

// Initialize the session/struct/program/file for the editor to use
void init_file(session *s, char **argv) {
    FILE *fd;
    s->fname = file_handler(argv, &fd);
    s->file = NULL;
    s->col_offset = 0;
    s->row_offset = 0;
    s->row = s->col = 0;
    read_file(fd, &s->nlines, &s->file);
    s->dirty = 0;
}

