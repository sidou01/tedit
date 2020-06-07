#include <curses.h>
#include <string.h>
#include <stdlib.h>

/* defines */
#define EDITOR "tedit"
#define VERSION "v0.1"

/* editor commands */
#define KEY_ESCAPE 27
#define KEY_EXIT_EDITOR "^X"


/* Draw the header and move the cursor to (1,0) */
void draw_header(const char *filename)
{
    attron(A_STANDOUT);
    int name_len = COLS/2 - (strlen(EDITOR) + strlen(VERSION) + 1);
    printw("%s %s %*s", EDITOR, VERSION, name_len, filename);
    printw("%*s", COLS/2, "");
    attroff(A_STANDOUT);
}

void process_keys(int *position_y, int *position_x, int key)
{
    switch(key)
    {
        case KEY_UP:
            if(*position_y > 1)
                *position_y = *position_y - 1;
            break;
        case KEY_DOWN:
            if(*position_y < LINES - 1)
                *position_y = *position_y + 1;
            break;
        case KEY_LEFT:
            if(*position_x > 0)
                *position_x = *position_x - 1;
            break;
        case KEY_RIGHT:
            if(*position_x < COLS - 1)
                *position_x = *position_x + 1;
            break;
    }
}

void init_editor()
{
    int key;
    int position_x = 0;
    int position_y= 1;
    while(1)
    {
        move(position_y, position_x);
        key = getch();
        process_keys(&position_y, &position_x, key);

        const char *key_name = keyname(key);
        if(strcmp(key_name, KEY_EXIT_EDITOR) == 0)
        {
            endwin();
            exit(0);
        }
    }

}

int main(int argc, char *argv[])
{

    char *filename;
    initscr();
    noecho();
    keypad(stdscr, TRUE);

    if(argv[1])
        filename = argv[1];
    else
        filename = "<<new file>>";

    draw_header(filename);
    init_editor();

    return 0;
}
