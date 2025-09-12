#include "session.h"
#include "state.h"
#include "input.h"
#include <unistd.h>

int main(int argc, char *argv[]) {
    char c;  // Used in the edit screen loop as input char
    session s;
    init_file(&s, argv);
   
    // Enter the terminal into rawMode for single char processing 
    enableEditMode();

    // Main loop 
    while (1) {
        print_to_screen(&s);   // Print the file to screen
        input_manager(&s, c);  // Run the input char through the gauntlet
    }
    return 0;
}
