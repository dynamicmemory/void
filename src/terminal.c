#include <termios.h>
#include <unistd.h>
#include <stdlib.h>

struct termios original_terminal;

// Called when program is exited, restores terminal settings to before running editor
void disableRaw() {
    tcsetattr(STDIN_FILENO, TCSANOW, &original_terminal);
}

/*
 * Switches of all controls in the terminal once the editor is running, this is 
 * so we can control every piece of input and map it to what we want it to do
 */
void enableRaw() {
    tcgetattr(STDIN_FILENO, &original_terminal);
    atexit(disableRaw);

    struct termios raw = original_terminal;
    // Disables auto echoing of keys, line buffering input, ctrl+c,z,o,v
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN); 
    // Disables ctrl+s,q, returns, newlines, interupts, stripping of 8bit inputs 
    raw.c_iflag &= ~(IXON | ICRNL | BRKINT | INPCK | ISTRIP);
    // Disables output processing 
    raw.c_oflag &= ~(OPOST);
    // Sets 8bit chars to ensure full utf8
    raw.c_cflag |= (CS8);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;


    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}




