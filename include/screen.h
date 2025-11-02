#ifndef SCREEN_H 
#define SCREEN_H 

#include <unistd.h>
#include "../include/document.h"
#include "../include/input.h"

/* 
 * screen.h
 *
 * Defines the screen/display managment for the text editor:
 * - init_screen: initializes the screen buff, its size and max capacity. 
 * - render_screen: Builds the screen line by line then writes it to the terminal. 
 * - update_viewport: Updates the screens visible rows and cols for display
 * - adjust_buff_size: Recalculates and allocs memory for the buffer 
 * - write_to_buff: Writes the char/line passed in to the screens buffer 
 * - render_status_bar: Takes care of building the status bar for the bottom row
 * - reset_screen: Resets the size of the screen to 0 for the next render cycle
 */ 

typedef struct {
    size_t cap;
    size_t size;
    char *buff; 
    int screen_rows;
    int screen_cols;
} screen;

// Move to display.c file when finished
void init_screen(screen *s);
void render_screen(document *d, editor *e, screen *s);
void update_viewport(editor *e, screen*s);
void adjust_buff_size(screen *s, int limit);
void write_to_buff(screen *s, char *data, int len);
void render_status_bar(document *d, editor *e, screen *s);
void reset_screen(screen *s);

#endif
