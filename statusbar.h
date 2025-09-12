#ifndef STATUSBAR_H 
#define STATUSBAR_H 

#include <stdio.h>
#include "session.h"

void get_window_size(int *rows, int *cols);
void draw_status_bar(session *s);
void viewport(session *s);

#endif
