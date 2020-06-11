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

/*
struct editor_state {
    int current_line;
    int term_height;
    int term_width;
    FILE *file_open;
};
*/

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
        default:
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

    while(1)
    {
        move(position_y, position_x);
        key = getch();
        process_keys(&position_y, &position_x, key);
        const char *key_name = keyname(key);
        process_editor_commands(key_name);
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

    /*
        fseek(file, 0, SEEK_END);
        file_size = ftell(file);
        fseek(file, 0, SEEK_SET);

        file_content = malloc(sizeof *file_content * file_size);
        if(file_content == NULL) { printf("Error allocating memory to a file\n"); exit(1); }
        fread(file_content, sizeof(char), file_size, file);
        printw("%s", file_content);
        free(file_content);
    */
    int y = 1;
    while(fgets(line, MAX_LINE_LENGTH, file) != NULL && y < LINES)
    {
        mvprintw(y++, 4, "%s", line);
    }
    /*
       Need to save lines printed in some sort of global struct (state)
       Print the rest of the lines as you scroll
    */
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

/* Need a global state */
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


    /* editor inits */
    draw_header(filename);
    read_file(filename);
    draw_line_numbers();

    edit();
    return 0;
}
