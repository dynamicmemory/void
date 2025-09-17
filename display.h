#ifndef DISPLAY_H
#define DISPLAY_H

#include "session.h"
#include <stdlib.h>

void update_viewport(session *s, int screen_rows, int screen_cols);
void build_statusbar(session *s, char *outbuff, size_t off, int screen_cols);
void print_to_screen(session *s);
void cursor_handler(session *s);

#endif
