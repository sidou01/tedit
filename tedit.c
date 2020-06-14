#include <curses.h>
#include <string.h>
#include <stdlib.h>

/* defines */
#define EDITOR "tedit"
#define VERSION "v0.1"
#define MAX_LINE_LENGTH 1000

/* editor commands */
#define KEY_ESCAPE 27
#define KEY_EXIT_EDITOR "^X"

struct file_t {
    FILE* file;
    char current_line[MAX_LINE_LENGTH];
    int cursor_x;
    int cursor_y;
};

void draw_header(const char *filename)
{
    attron(COLOR_PAIR(2));
    int name_len = COLS/2 - (strlen(EDITOR) + strlen(VERSION) + 1);
    printw("%s %s %*s", EDITOR, VERSION, name_len, filename);
    printw("%*s", COLS/2, "");
    attroff(COLOR_PAIR(1));
}

/* interpret arrow/command keys and print the rest */
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
        case 127: //127 = backspace
            /* Remove last char */
            *position_x = *position_x - 1;
            break;
        default:
            *position_x = *position_x + 1;
            addch(key);
    }
}

void process_editor_commands(const char *key_name)
{
    /* vim commands */
    if(strcmp(key_name,":") == 0)
    {
        const char *next_key = keyname(getch());
        switch(*next_key)
        {
            case 'q':
                endwin();
                exit(1);
                break;
            case 'w':
                /* Get the FILE* as param or just use the global struct (state) and save changes */
                break;

        }
    }
    /* or can exit with ^X */
    if(strcmp(key_name, KEY_EXIT_EDITOR) == 0)
    {
        endwin();
        exit(0);
    }
}

void edit()
{
    int key;
    int position_x = 4;
    int position_y= 1;

    move(position_y, position_x);
    while(1)
    {
        key = getch();
        process_keys(&position_y, &position_x, key);
        const char *key_name = keyname(key);
        process_editor_commands(key_name);
        move(position_y, position_x);
    }
}

void read_file(char *filename)
{
    FILE *file;
    char *file_content;
    char line[MAX_LINE_LENGTH];
    int file_size;

    if((file = fopen(filename, "r+")) == NULL)
    { 
        if((file = fopen(filename, "w+")) == NULL)
        {
            endwin();
            printf("Error opening the file \"%s\"\n", filename);
            exit(1);
        }
    }
    int y = 1;
    while(fgets(line, MAX_LINE_LENGTH, file) != NULL && y < LINES)
    {
        mvprintw(y++, 4, "%s", line);
    }
}

void draw_line_numbers()
{
    for(int i = 1; i < LINES; i++)
    {
        move(i,0);
        attron(COLOR_PAIR(1));
        printw("%d", i);
        attroff(COLOR_PAIR(1));
    }
}

void init_colors()
{
    if(has_colors())
    {
        start_color();
        init_pair(1, COLOR_GREEN, COLOR_BLACK);
        init_pair(2, COLOR_BLACK, COLOR_GREEN);
    }
    else
    {
        endwin();
        printf("Your terminal does not support colors!");
        exit(1);
    }
}

int main(int argc, char *argv[])
{
    char *filename;

    initscr();
    noecho();
    keypad(stdscr, TRUE);
    init_colors();

    if(argv[1])
        filename = argv[1];
    else
        filename = "<<new file>>";


    draw_header(filename);
    read_file(filename);
    draw_line_numbers();

    edit();
    return 0;
}
