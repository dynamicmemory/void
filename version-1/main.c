#include "session.h"
#include "sysops.h"
#include "input.h"
#include "display.h"
#include "resize.h"
#include <unistd.h>

int main(int argc, char *argv[]) {
    char c;  // Used in the edit screen loop as input char
    session s;
    init_file(&s, argv);
    install_winch_handler();

    // Enters terminal into raw mode 
    enableEditMode();

    while (1) {
        // Print the screen if there has been a resize
        if (get_resized()) {
            clear_resized_flag();
            print_to_screen(&s);
        }

        // Reads a single key input or gets interupted to resize
        int c = read_key();
        if (c == -1) continue;   // Got interupted, resize and start again.

        // Business as usual
        input_manager(&s, (char)c); 
        print_to_screen(&s);
    }
    return 0;
}
