#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
// resize deps
#include <stdio.h>
#include <signal.h>
#include <errno.h>

#include <sys/ioctl.h>

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

// --------------------- Screen size calulation code ----------------------

/* Gets the current size of the terminal window and saves it to the screens 
 * row and col values */
void get_window_size(int *rows, int *cols) {
    struct winsize ws; 

    if (ioctl(STDOUT_FILENO , TIOCGWINSZ, &ws) == -1) {
        perror("Error: ioctrl - unable to get window");
        *rows = 24;
        *cols = 80;
    }
    else {
        *rows = ws.ws_row;
        *cols = ws.ws_col;
    }
}

// -------------------- Screen resize code ------------------------

// Flag that alerts program to window resize, safe to modify in signal handler
volatile sig_atomic_t resized = 1;

// Sets the above flag for SIGWINCH (window has changed)
static void handle_winch(int signo) {
    (void)signo;
    resized = 1;
}

// "installs" the SIGWINCH so we are informed when terminal window changes 
void install_winch_handler(void) {
    struct sigaction sa = {0};
    sa.sa_handler = handle_winch;
    sigemptyset(&sa.sa_mask);

    // No SA_RESTART flag, so blocking syscalls (read) will be interuptted 
    // and return with errno = EINTR - this allows is to break out and redraw
    if (sigaction(SIGWINCH, &sa, NULL) == -1) {
        perror("sigaction(SIGWINCH)");
        exit(1);
    }
}

// Reads a single key from stdin. Safe in presence of signals because 
// we detect EINTR (interupted syscalls) and let the caller handle it.
int read_key(void) {
    char c;
    ssize_t n = read(STDIN_FILENO, &c, 1);
    if (n == 1) return (unsigned char)c;         // Normal case, got a key 
    if (n == 0) return -1;                       // EOF (CTRL + D)
    if (n == -1 && errno == EINTR) return -1;    // interupted by signal
    perror("read");
    exit(1);
}

// Helper for main to get access to resized
int get_resized(void) {
    return resized;
}

// Helper to reset resized if screen size has changed.
void clear_resized_flag(void) {
    resized = 0;
}


