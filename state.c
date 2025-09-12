#include "termios.h"
#include "unistd.h"
#include "stdlib.h"

// Stores the state of the terminal for when you exit void
struct termios orig_termios; 

void disableEditMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enableEditMode() {
    // Get current terminal state 
    tcgetattr(STDIN_FILENO, &orig_termios);

    // Remember to set the terminal back to normal on exit 
    atexit(disableEditMode);

    // Create a new struct for this text editing session
    struct termios edit = orig_termios;

    // All the flags needed to disable cannonical mode in the terminal
    edit.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
    edit.c_iflag &= ~(IXON | ICRNL | BRKINT | INPCK | ISTRIP);
    edit.c_oflag &= ~(OPOST);
    edit.c_cflag |= CS8;
    edit.c_cc[VMIN] = 1;
    edit.c_cc[VTIME] = 0;

    // Set the terminals attributes to the new flags
    tcsetattr(STDIN_FILENO, TCSAFLUSH ,&edit);
}
