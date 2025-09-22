#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

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
