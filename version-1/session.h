#ifndef SESSION_H
#define SESSION_H 

#define CTRL_Q 17 
#define CTRL_S 19 

typedef struct session {
    char *fname;
    char **file;
    int nlines;
    int row;
    int col;
    int row_offset;
    int col_offset;
    int dirty;
} session;

void init_file(session *s, char **arg);

#endif
