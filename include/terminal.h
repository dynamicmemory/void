#ifndef TERMINAL_H 
#define TERMINAL_H 

/* 
 * terminal.h
 *
 * Defines jobs for the terminal/system to look after :
 * - disableRaw: Move the terminal from raw edit mode back into concat mode.  
 * - enableRaw: Moves the terminal from concat mode into raw mode 
 *
 */ 

// Termios function headers
void disableRaw();
void enableRaw();

void get_window_size(int *r, int *c);

// Resize function headers 
void install_winch_handler(void);
int read_key(void);
int get_resized(void);
void clear_resized_flag(void);

#endif 
