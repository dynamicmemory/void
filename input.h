#ifndef INPUT_H 
#define INPUT_H 

#include "session.h"

#define CTRL_Q 17 
#define CTRL_S 19 

void clear_screen();
void move_cursor(int row, int col);
void print_to_screen(session *s);
void cursor_handler(session *s);
void insert_char(session *s, char c);
void backspace(session *s);
void newline(session *s);
void input_manager(session *s, char c);

#endif
