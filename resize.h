#ifndef RESIZE_H 
#define RESIZE_H

static void handle_winch(int signo);
void install_winch_handler(void);
int read_key(void);
int get_resized(void);
void clear_resized_flag(void);

#endif
