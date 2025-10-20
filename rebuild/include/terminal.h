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

void disableRaw();
void enableRaw();

#endif 
