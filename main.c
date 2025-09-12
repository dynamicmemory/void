#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "termios.h"
#include "unistd.h"

#define CTRL_Q 17 
#define CTRL_S 19 

typedef struct session {
    char *fname;
    char **file;
    int nlines;
    int c_row;
    int c_col;
} session;

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

// Clears the terminal screen completely
void clear_screen() { printf("\x1b[2J"); }

// Moves the cursor to the given row and column
void move_cursor(int row, int col) { printf("\x1b[%d;%dH", row, col); }

// Reprints the screen after every char input
void print_to_screen(char **flines, int rownums, int row, int col) {
    clear_screen(); // Clear the screen 
    move_cursor(0, 0); // Move the cursor to the top left to reprint all lines 
    for (int i = 0; i < rownums; i++) {
        printf("%s\r\n", flines[i]); 
    }
    move_cursor(row + 1, col + 1); // Adding one to both for 0 index offset
    fflush(stdout);
}

// If a file isnt provided as an argument, then this function creates a file 
// with our predetermined naming convention.
char *create_default_file() {
    static char fname[256];
    int index = 0;

    // Loops until we find a filename that doesn't exist, returns that name 
    while (1) {
        if (index == 0) 
            snprintf(fname, sizeof(fname), "newvoid.txt");
        else 
            snprintf(fname, sizeof(fname), "newvoid%d.txt", index);

        FILE *fd = fopen(fname, "r");
        if (!fd) 
            break;
        
        fclose(fd);
        index++;
    }
    return fname;
}

// Opens provided file or creates one if needed
char *file_handler(char **arg, FILE **fd) {
    char *fname;
    // If an argument was provided then try open the file 
    if (arg[1]) {
        fname = arg[1];
        *fd = fopen(fname, "r");    
    
        // If the file failed to open, try to create it with the name 
        if (!*fd) *fd = fopen(fname, "w");
        // If the file could not be created exit according
        if (!*fd) {
            perror("Error: Unable to create file");
            exit(1);
        }
    }
    // If no file name was provide, call create a file
    else {
        fname = create_default_file();
        *fd = fopen(fname, "w");

        if (!*fd) {
            perror("Error: Unable to create file");
            exit(1);
        }
    }
    return fname;
}

void save_file(char **lines, char *fname, int nlines) {
    // Open the file, get the fd and format print it 
    FILE *fd = fopen(fname, "w");
    if (fd) {
        for (int i = 0; i < nlines; i++)
            fprintf(fd, "%s\n", lines[i]);
    }
    else 
        perror("Error: failed to open the file for saving");
}

// Reads through the provided fd and saves it all to a 2d array 
void read_file(FILE *fd, int *nlines, char ***file) {
    char *line = NULL; 
    size_t len = 0;
    while (getline(&line, &len, fd) != -1) {
        size_t linelen = strlen(line);

        // Removes the \n from the end of all lines to avoid diagonal printing 
        if (linelen > 0 && line[linelen - 1] == '\n') line[linelen - 1] = '\0'; 

        // Resize and reallocate the array in memory for each line added 
        char **temp = realloc(*file, sizeof(char*) * (*nlines + 1));
        // Check for allocation failure
        if (!temp) { 
            perror("Error: realloc failed"); 
            exit(1); 
        }

        *file = temp;  // Assign reallo to file.

        // Duplicate the line added or else each pointer will overwrite the last 
        (*file)[*nlines] = strdup(line);  // Deref, then get line
        // Check for duplcation failure
        if (!(*file)[*nlines]) { 
            perror("Error: String duplcation failed"); 
            exit(1); 
        }

        (*nlines)++;  // Deref, then increment
    }
    free(line);
    fclose(fd);

    if (*nlines == 0) {
        *file = malloc(sizeof(char*));
        (*file)[0] = strdup("");  // Deref, then get line
        if (!(*file)[0]) { 
            perror("Error: String duplcation failed"); 
            exit(1);
        }

        (*nlines)++;   // defer, then increment
    }
}

// Initialize the session/struct/program/file for the editor to use
void init_file(session *s, char **argv) {
    FILE *fd;
    s->fname = file_handler(argv, &fd);
    s->file = NULL;
    read_file(fd, &s->nlines, &s->file);
    s->c_row = s->c_col = 0;
}

void cursor_handler(session *s) {
    char seq[2];

    // Two chars for an arrow press, not just one
    if (read(STDIN_FILENO, &seq[0], 1) != 1) return;
    if (read(STDIN_FILENO, &seq[1], 1) != 1) return;
    // Add or subtract one for the cursor movement
    if (seq[0] == '[') {
        switch(seq[1]) {
            case 'A': if (s->c_row > 0) {
                        s->c_row--;
                        break;
            }
            case 'B': if (s->c_row < s->nlines -1) {
                        s->c_row++;
                        break;
            }
            case 'C': s->c_col++; break;
            case 'D': if (s->c_col > 0) {
                        s->c_col--;
                        break;
            }
        }
    }
}

void insert_char(session *s, char c) {
    char *line = s->file[s->c_row];
    size_t len = strlen(line);

    // Make room for the new char
    s->file[s->c_row] = realloc(line, len + 2);

    memmove(&s->file[s->c_row][s->c_col + 1], 
            &s->file[s->c_row][s->c_col], len - s->c_col + 1);

    s->file[s->c_row][s->c_col] = c;
    s->c_col++;
}

void backspace(session *s) {
    // If the line has at least one char 
    if (s->c_col > 0) {
    // Get the current line 
        char *line = s->file[s->c_row];
        size_t len = strlen(line);

        // Move the char one spot ahead back one
        memmove(&line[s->c_col - 1], &line[s->c_col], len - s->c_col + 1);
            s->c_col--;    
    }
                
    // If there are no chars but there are multiple lines, remove the line
    if (s->c_col == 0 && s->c_row > 0) {
        int prevlen = strlen(s->file[s->c_row - 1]);
        char *temp = realloc(s->file[s->c_row - 1], 
                              prevlen + strlen(s->file[s->c_row]) + 1);

        if (!temp) { 
            perror("Error: realloc failed"); 
            exit(1); 
        }
        s->file[s->c_row - 1] = temp;

        // Stitch the previous line and current line together
        strcat(s->file[s->c_row - 1], s->file[s->c_row]);

        // free the current line from memory as its about to become the prev line
        free(s->file[s->c_row]);

        memmove(&s->file[s->c_row], &s->file[s->c_row + 1], 
                sizeof(char*) * (s->nlines - s->c_row - 1));
        s->nlines--;
        s->c_row--;
        s->c_col = prevlen;
    }
}

void newline(session *s) {
    // Get the current line and the points that will be split to the left and right
    char *current = s->file[s->c_row];
    char *left = strndup(current, s->c_col);
    if (!left) {
        perror("Error: strnup failed");
        exit(1);
    }

    char *right = strdup(current + s->c_col);
    if (!right) {
        perror("Error: strnup failed");
        exit(1);
    }

    char **temp = realloc(s->file, sizeof(char*) * (s->nlines + 1));
    if (!temp) { 
        perror("Error: realloc failed"); 
        exit(1); 
    }
    s->file = temp;

    // Move the line one exttra line down in memory 
    memmove(&s->file[s->c_row + 2], &s->file[s->c_row + 1], 
            sizeof(char*) * (s->nlines - s->c_row - 1));

    // current line now just equals left
    s->file[s->c_row] = left;
    // New line equals right 
    s->file[s->c_row + 1] = right;
    s->nlines++;
    s->c_row++;
    s->c_col = 0;
}

void input_manager(session *s, char c) {
    if (c == CTRL_Q) exit(0);
    if (c == CTRL_S) save_file(s->file, s->fname, s->nlines);
    // Handles where the cursor is and the arrow key presses
    if (c == '\x1b') cursor_handler(s);
    // Clamp x axis so we cant keep going ad inifinitum on the x axis 
    if (s->c_col > strlen(s->file[s->c_row])) 
        s->c_col = strlen(s->file[s->c_row]);
    // Handles inserting any char 
    if (c >= 32 && c <= 126) insert_char(s, c);
    if (c == 127) backspace(s);
    if (c == '\n' || c == '\r') newline(s);
}

int main(int argc, char *argv[]) {
    char c;  // Used in the edit screen loop as input char
    session s;
    init_file(&s, argv);

    // Enter the terminal into rawMode for single char processing 
    enableEditMode();

    // Print the file to the terminal once before entering the loop forever
    print_to_screen(s.file, s.nlines, s.c_row, s.c_col);

    // So this runs while the editor is open, it is the main program loop
    while (1) {
        // Read the char that was entered from the keyboard
        if (read(STDIN_FILENO, &c, 1) == 1) {
        input_manager(&s, c);
        // Print loop that repaints the file in the terminal each keystroke 
        print_to_screen(s.file, s.nlines, s.c_row, s.c_col);
        }
    }
    return 0;
}
