#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include "../include/document.h"
#include "../include/terminal.h"
#include "../include/input.h"
#include "../include/screen.h"

int main(int argc, char *argv[]) {
    // Set the terminal into raw mode 
    enableRaw();

    if (argc > 2) {
        perror("Usage: Only enter one filename");
        return 1;
    }
    document d;
    if (init_doc(&d, argv) == -1) {
        fprintf(stderr, "Failed to correctly handle file");
        return -1;
    }

    editor e;
    init_editor(&e);
    screen s; 
    init_screen(&s);
    render_screen(&d, &e, &s);

    char c;
    while (read(0, &c, 1) == 1) {
        input_handler(&d, &e, c);
        render_screen(&d, &e, &s);
    }
    return 0;
}
