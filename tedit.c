#include <curses.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

/* defines */
#define EDITOR "tedit"
#define VERSION "v0.1"
#define MAX_LINE_LENGTH 1000

/* editor commands */
#define KEY_ESCAPE 27
#define KEY_EXIT_EDITOR "^X"

struct file_t {
    FILE* file;
    const char *filename;
    char current_line[MAX_LINE_LENGTH];
    int cursor_x;
    int cursor_y;
};

void draw_header(const char *filename)
{
    /* time_t mytime = time(NULL); */
    /* char * time_str = ctime(&mytime); */
    /* time_str[strlen(time_str)-1] = '\0'; */

    attron(COLOR_PAIR(2));
    int name_len = COLS/2 - (strlen(EDITOR) + strlen(VERSION) +  1);
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

void edit(struct file_t *file)
{
    int key;

    move(file->cursor_y, file->cursor_x);
    while(1)
    {
        key = getch();
        process_keys(&file->cursor_y, &file->cursor_x, key);
        const char *key_name = keyname(key);
        process_editor_commands(key_name);
        move(file->cursor_y, file->cursor_x);
    }
}

void read_file(struct file_t *file)
{
    /* char *file_content; */
    char line[MAX_LINE_LENGTH];
    /* int file_size; */

    if((file->file = fopen(file->filename, "r+")) == NULL)
    { 
        if((file->file = fopen(file->filename, "w+")) == NULL)
        {
            endwin();
            printf("Error opening the file \"%s\"\n", file->filename);
            exit(1);
        }
    }
    int y = 1;
    while(fgets(line, MAX_LINE_LENGTH, file->file) != NULL && y < LINES)
    {
        /* copy the first line where the cursor is */
        if(y == 1) strcpy(file->current_line, line);
        mvprintw(y++, 4, "%s", line);
    }
    file->cursor_x = 4;
    file->cursor_y = 1;
}

void draw_line_numbers(void)
{
    for(int i = 1; i < LINES; i++)
    {
        move(i,0);
        attron(COLOR_PAIR(1));
        printw("%d", i);
        attroff(COLOR_PAIR(1));
    }
}

void init_colors(void)
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
    //should not be here. (don't know yet).
    struct file_t *file = malloc(sizeof *file);

    initscr();
    noecho();
    keypad(stdscr, TRUE);
    init_colors();

    if(argv[1])
        file->filename = argv[1];
    else
        file->filename = "<<new file>>";


    draw_header(file->filename);
    read_file(file);
    draw_line_numbers();

    edit(file);
    return 0;
}
