#include <stdio.h>
#include <unistd.h>
#include "../include/document.h"
#include "../include/terminal.h"
#include "../include/input.h"
#include "../include/screen.h"

int main(int argc, char *argv[]) {
    // Set the terminal into raw mode 
    enableRaw();
    install_winch_handler();

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

    while (1) {
        if (get_resized()) {
            clear_resized_flag();
            render_screen(&d, &e, &s);
        }

        // Reads a single key input or gets interupted to resize
        int c = read_key();
        if (c == -1) continue;

        input_handler(&d, &e, (char)c);
        render_screen(&d, &e, &s);

    }
    return 0;
}
