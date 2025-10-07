#include <stdio.h>
#include "../include/file_ops.h"

typedef struct file {
    char **lines;
    char *fname;
    int nlines;
    int capacity;
} file;

int init_editor(file *e, char **arg) {
    e->lines = NULL;
    e->capacity = 0;
    if (init_file(arg, e) == -1) return -1;
    if (read_file(e) == -1) return -1;

    return 0;
}
