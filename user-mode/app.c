#include <curses.h>
#include <stdlib.h>
#include <stdio.h>

#define NORMAL 1
#define HILITE 2

#define BUTTON_NONE 0
#define BUTTON_UP 1
#define BUTTON_DOWN 2
#define BUTTON_LEFT 3
#define BUTTON_RIGHT 4

static struct state_t {
    // model
    uint8_t field[12];

    // presentation / controller
    int running;
    int button;
    uint8_t selected_field;
} state = {
    192, 168, 50, 100, /* IPv4 */
    255, 255, 255, 255, /* Netmask */
    192, 168, 50, 1, /* Gateway */
    0 /* zero initialize the rest */
};



void color_string(int y, int x, const char *str, int color)
{
    attron(COLOR_PAIR(color));
    mvaddstr(y, x, str);
    attroff(COLOR_PAIR(color));
}

void add_octet_string(int y, int x, uint8_t v, int color)
{
    char str[4] = {0};
    snprintf(str, 4, "%03d", v);
    color_string(y, x, str, color);
}

int getinput() 
{
    int ch = getch();
    switch (ch) {
        case 'w':
            return BUTTON_UP;
        case 's':
            return BUTTON_DOWN;
        case 'a':
            return BUTTON_LEFT;
        case 'd':
            return BUTTON_RIGHT;
    }
    return BUTTON_NONE;
    // TODO: Consider flushing getch().
}

void process()
{
    if (state.button == BUTTON_RIGHT)
    {
        state.selected_field = (state.selected_field + 1) % 15;
    }

    if (state.selected_field == 14 && state.button == BUTTON_UP)
    {
        state.running = FALSE;
    }

    if (state.selected_field >= 0 && state.selected_field <= 11 && state.button == BUTTON_UP)
    {
        // increase value
        state.field[state.selected_field] += 1;
    }
}

void render()
{
    color_string(1, 1, "   IPv4: 000.000.000.000", NORMAL);
    add_octet_string(1, 10, state.field[0], state.selected_field == 0 ? HILITE : NORMAL);
    add_octet_string(1, 14, state.field[1], state.selected_field == 1 ? HILITE : NORMAL);
    add_octet_string(1, 18, state.field[2], state.selected_field == 2 ? HILITE : NORMAL);
    add_octet_string(1, 22, state.field[3], state.selected_field == 3 ? HILITE : NORMAL);
    color_string(3, 1, "Netmask: 000.000.000.000", NORMAL);
    add_octet_string(3, 10, state.field[4], state.selected_field == 4 ? HILITE : NORMAL);
    add_octet_string(3, 14, state.field[5], state.selected_field == 5 ? HILITE : NORMAL);
    add_octet_string(3, 18, state.field[6], state.selected_field == 6 ? HILITE : NORMAL);
    add_octet_string(3, 22, state.field[7], state.selected_field == 7 ? HILITE : NORMAL);
    color_string(5, 1, "Gateway: 000.000.000.000", NORMAL);
    add_octet_string(5, 10, state.field[8], state.selected_field == 8 ? HILITE : NORMAL);
    add_octet_string(5, 14, state.field[9], state.selected_field == 9 ? HILITE : NORMAL);
    add_octet_string(5, 18, state.field[10], state.selected_field == 10 ? HILITE : NORMAL);
    add_octet_string(5, 22, state.field[11], state.selected_field == 11 ? HILITE : NORMAL);

    color_string(7, 1, "Save", state.selected_field == 12 ? HILITE : NORMAL);
    color_string(7, 7, "Revert", state.selected_field == 13 ? HILITE : NORMAL);
    color_string(7, 15, "Reboot", state.selected_field == 14 ? HILITE : NORMAL);

    refresh();
}

int main(void)
{

    FILE *tty_file = fopen("/dev/tty0", "r+");
    //fwrite("mine\0", 5, 1, tty_file);
    //fflush(tty_file);
    SCREEN *tty_scr = newterm("xterm-16color", tty_file, stdin); 
	/* initialize curses */
    state.running = TRUE;
    state.selected_field = 0;
    WINDOW *init_win = initscr();
    SCREEN *init_scr = set_term(tty_scr);
    WINDOW *win = initscr();

    //set_term(tty_scr);
    //keypad(win, TRUE);
    timeout(-1);
    start_color();
    init_pair(HILITE, COLOR_BLACK, COLOR_WHITE);
    init_pair(NORMAL, COLOR_WHITE, COLOR_BLACK);
    cbreak();
    noecho();
    clear();

    while (state.running)
    {
        render();
        state.button = getinput();
        process();
    }
    
    set_term(init_scr);
    nocbreak();
    echo();
    endwin();
    exit(0);
}
