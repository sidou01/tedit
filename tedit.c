#include <curses.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define EDITOR "tedit"
#define VERSION "v0.1"
#define MAX_LINE_LENGTH sizeof(char) * 1000
#define MAX_NUMBER_OF_LINES 10000

#define KEY_ESCAPE 27
#define KEY_EXIT_EDITOR "^X"

struct file_t {
    FILE* file;
    const char *filename;
    char *file_content[MAX_NUMBER_OF_LINES];
};

struct cursor_t {
    int cursor_x;
    int cursor_y;
};

void draw_header(const char *filename)
{
    attron(COLOR_PAIR(2));
    int name_len = COLS/2 - (strlen(EDITOR) + strlen(VERSION) +  1);
    printw("%s %s %*s", EDITOR, VERSION, name_len, filename);
    printw("%*s", COLS/2, "");
    attroff(COLOR_PAIR(1));
}

/* TODO: insert a char at k position on a char array */
char *insert_char_at(char *string, char ch, int position)
{
    int length = strlen(string);

    if(position > length)
    {
        int i = length;
        while(i < position)
            string[i++] = ' ';
    }
    else
    {
        string[length + 1] = '\0';
        while(length > position) {
            string[length] = string[length - 1];
            length--;
        }
    }
    string[position] = ch;
    return string;
}

/* Need to handle Backspace at the beiginning of the line (join current line with previous one). */
char *delete_from_string_at(char *string, int position)
{
    int length = strlen(string) - 1;
    if(position > 0)
    {
        position = position - 1;
        while(position < length)
        {
            string[position] = string[position+1];
            position++;
        }
        string[length] = ' ';
    }
    return string;
}

/* interpret arrow/command keys and print the rest */
void process_keys(int *position_y, int *position_x, int key, struct file_t *file)
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
            if(*position_x > 4)
                *position_x = *position_x - 1;
            break;
        case KEY_RIGHT:
            if(*position_x < COLS - 1)
                *position_x = *position_x + 1;
            break;
        case 127: //127 = backspace
            file->file_content[*position_y - 1] = delete_from_string_at(file->file_content[*position_y - 1], *position_x - 4);
            mvaddstr(*position_y, 4, file->file_content[*position_y - 1]);
            if(*position_x > 4)
                *position_x = *position_x - 1;
            break;
        default:
            file->file_content[*position_y - 1] = insert_char_at(file->file_content[*position_y - 1], key, *position_x - 4);
            mvaddstr(*position_y, 4, file->file_content[*position_y - 1]);
            *position_x = *position_x + 1;
            move(*position_y, *position_x);
    }
    
}

void free_mem(struct file_t *file, struct cursor_t *cursor)
{
    /* Free allocated memeory */
    fclose(file->file);
    for(int i = 0; i < MAX_NUMBER_OF_LINES; i++)
        free(file->file_content[i]);

    free(cursor);
}
void process_editor_commands(const char *key_name, struct file_t *file, struct cursor_t *cursor)
{
    /* Exit with :q */
    if(strcmp(key_name,":") == 0)
    {
        const char *next_key = keyname(getch());
        switch(*next_key)
        {
            case 'q':
                endwin();
                free_mem(file, cursor);
                exit(0);
                break;
            case 'w':
                /* save the file with snprintf */
                break;

        }
    }
    if(strcmp(key_name, KEY_EXIT_EDITOR) == 0) //^X
    {
        endwin();
        exit(0);
    }
}

void edit(struct file_t *file, struct cursor_t *cursor)
{
    int key;

    move(cursor->cursor_y, cursor->cursor_x);
    while(1)
    {
        key = getch();
        process_keys(&cursor->cursor_y, &cursor->cursor_x, key, file);
        const char *key_name = keyname(key);
        process_editor_commands(key_name, file, cursor);
        move(cursor->cursor_y, cursor->cursor_x);
    }
}

void read_file(struct file_t *file, struct cursor_t *cursor)
{
    if((file->file = fopen(file->filename, "r+")) == NULL)
    { 
        if((file->file = fopen(file->filename, "w+")) == NULL)
        {
            endwin();
            printf("Error opening the file \"%s\"\n", file->filename);
            exit(1);
        }
    }
    /* might use lines_count at some point */
    int lines_count = 0;
    char c;
    for (c = getc(file->file); c != EOF; c = getc(file->file)) 
        if (c == '\n') // Increment count if this character is newline 
            lines_count = lines_count + 1;

    /* go back to the beginning of the file */
    /* to save the lines and print them to the screen. */
    rewind(file->file);
    for(int t = 0; t < LINES; t++)
    {
        file->file_content[t] = malloc(MAX_LINE_LENGTH);
    }
    int i = 0;
    while(fgets(file->file_content[i], MAX_LINE_LENGTH, file->file) != NULL)
    {
        file->file_content[i][strlen(file->file_content[i]) - 1] = '\0';
        mvaddstr(i+1, 4, file->file_content[i]);
        i++;
    }
    fclose(file->file);
    cursor->cursor_x = 4;
    cursor->cursor_y = 1;
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
    struct file_t *file = malloc(sizeof *file);
    struct cursor_t *cursor = malloc(sizeof *cursor);

    initscr();
    noecho();
    keypad(stdscr, TRUE);
    init_colors();

    if(argv[1])
        file->filename = argv[1];
    else
        file->filename = "<<new file>>";


    draw_header(file->filename);
    read_file(file, cursor);
    draw_line_numbers();

    edit(file, cursor);

    return 0;
}
