// TODO: Turning off and on cursor to stop it from flickering during page rewrite
// TODO: Using a buffer to write changes rather then making sys calls 
//       endlessly per input 
// TODO: Make sure all lines are null terminated as writing past col limit on 
//       a single line causes random chars to appear on other lines.

#include "session.h"
#include "state.h"
#include "input.h"
#include <unistd.h>

int main(int argc, char *argv[]) {
    char c;  // Used in the edit screen loop as input char
    session s;
    init_file(&s, argv);
   
    // Enter the terminal into rawdawg mode for single char processing 
    enableEditMode();

    print_to_screen(&s);   // Print the file to screen
    // Main loop 
    while (1) {
        input_manager(&s, c);  // Run the input char through the gauntlet
        print_to_screen(&s);   // Print the file to screen
    }
    return 0;
}
