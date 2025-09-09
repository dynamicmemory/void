#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "termios.h"
#include "unistd.h"

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


int main(int argc, char *argv[]) {
    char **filelines = NULL; 
    char *filename;
    int numline = 0;
    FILE *fd;

    // Get the file from the command line 
    if (argc > 1) {
        // save the filename to a variable
        filename = argv[1]; 
        fd = fopen(filename, "r");
         
    } 
    // User forgot to add file in args list on launch
    else {
        fprintf(stderr, "Error: No file provided\n");
        return 1;
    }

    // fd is not a valid file pointer
    if (fd == NULL) {
        fprintf(stderr, "Error: Could not open file");
        return 1;
    }

    // Iterate through all lines of the file, saving them to a 2d array and 
    // counting the number of lines
    char *line = NULL;
    size_t len = 0;
    while (getline(&line, &len, fd) != -1) {

        // Removes \n chars from the end of all lines to avoid diagonal printing
        size_t linelen = strlen(line);
        if (linelen > 0 && line[linelen - 1] == '\n') {
            line[linelen - 1] = '\0';
        }

        // reallocating the 2d array with the new added size 
        filelines = realloc(filelines, sizeof(char*) * (numline + 1));

        // duplicate the line into its correct number in the array (must do or 
        // else overwrite "line" in the next iteration)
        filelines[numline] = strdup(line);
        numline++;
    }
    free(line);
    fclose(fd);

    // Print each line to the terminal
    for (int i = 0; i < numline; i++) 
        printf("%s", filelines[i]);



    // Enter the terminal into rawMode for single char processing 
    enableEditMode();
    
    char c; 
    int cx = 0;  // Column
    int cy = 0;  // Row

    // Print the file to the terminal once before entering the loop forever
    printf("\x1b[2J");
    printf("\x1b[H");
    for (int i = 0; i < numline; i++)
        printf("%s\r\n", filelines[i]);
    printf("\x1b[%d;%dH", cy + 1, cx + 1);
    fflush(stdout);

    // So this runs while the editor is open, it is the main program loop
    while (1) {
        // Read the char that was entered from the keyboard
        if (read(STDIN_FILENO, &c, 1) == 1) {

            // Handles quiting, will change later to no letter char
            if (c == 17) break; 

            // Hanfles saving the file 
            if (c == 19) {
                // Open the file, get the fd and format print it 
                FILE *fd = fopen(filename, "w");
                if (fd) {
                    for (int i = 0; i < numline; i++)
                        fprintf(fd, "%s\n", filelines[i]);
                }
                else {
                    perror("Error: failed to open the file for saving");
                }
            }

            // Handles where the cursor is and the arrow key presses
            if (c == '\x1b') {
                char sequence[2];
                // Two chars for an arrow press, not just one
                if (read(STDIN_FILENO, &sequence[0], 1) != 1) continue;
                if (read(STDIN_FILENO, &sequence[1], 1) != 1) continue;

                // Add or subtract one for the cursor movement
                if (sequence[0] == '[') {
                    switch(sequence[1]) {
                        case 'A': if (cy > 0) {
                                      cy--;
                                      break;
                                  }
                        case 'B': if (cy < numline-1) {
                                      cy++;
                                      break;
                                  }
                        case 'C': cx++; break;
                        case 'D': if (cx > 0) {
                                      cx--;
                                      break;
                                  }
                    }
                }
            }

            // Clamp x axis so we cant keep going ad infinitum on the x axis
            if (cx > strlen(filelines[cy])) 
                cx = strlen(filelines[cy]);

            // Hangles char inserting
            if (c >= 32 && c <= 126) {
                char *line = filelines[cy];
                size_t len = strlen(line);

                // Make room for the new char
                filelines[cy] = realloc(line, len + 2);

                memmove(&filelines[cy][cx + 1], &filelines[cy][cx], len - cx + 1);
                filelines[cy][cx] = c;
                cx++;
            }

            // Handles back spaces
            if (c == 127) {
                // If the line has at least one char 
                if (cx > 0) {
                    // Get the current line 
                    char *line = filelines[cy];
                    size_t len = strlen(line);

                    // Move the char one spot ahead back one
                    memmove(&line[cx - 1], &line[cx], len - cx + 1);
                    cx--;    
                }
                
                // If there are no chars but there are multiple lines, remove the line
                if (cx == 0 && cy > 0) {
                    int prevlen = strlen(filelines[cy - 1]);
                    filelines[cy - 1] = realloc(
                        filelines[cy - 1], 
                        prevlen + strlen(filelines[cy]) + 1
                    );

                    // Stitch the previous line and current line together
                    strcat(filelines[cy - 1], filelines[cy]);

                    // free the current line from memory as its about to become the prev line
                    free(filelines[cy]);

                    memmove(&filelines[cy], &filelines[cy + 1], sizeof(char*) * (numline - cy - 1));
                    numline--;
                    cy--;
                    cx = prevlen;
                }
            }

            // Handles newlines
            if (c == '\n' || c == '\r') {

                // Get the current line and the points that will be split to the left and right
                char *current = filelines[cy];
                char *left = strndup(current, cx);
                char *right = strdup(current + cx);

                filelines = realloc(filelines, sizeof(char*) * (numline + 1));
                // Move the line one exttra line down in memory 
                memmove(&filelines[cy + 2], &filelines[cy + 1], sizeof(char*) * (numline - cy - 1));

                // current line now just equals left
                filelines[cy] = left;
                // New line equals right 
                filelines[cy + 1] = right;
                numline++;
                cy++;
                cx = 0;
            }
            // Print loop that repaints the file in the terminal each keystroke 
            printf("\x1b[2J");  // celar the screen
            printf("\x1b[H");   // reset the cursor 
            for (int i = 0; i < numline; i++)
                printf("%s\r\n", filelines[i]);
            printf("\x1b[%d;%dH", cy + 1, cx + 1); // Move the cursor 
            fflush(stdout);

            // write(STDOUT_FILENO, &c, 1);

        }
    }

    return 0;
}
