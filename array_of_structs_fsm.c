#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>

//---Custom Types---
typedef enum {
    STATE_MENU_NAV,
    STATE_OPTION_NAV,
    STATE_OPTION_EDIT,
    STATE_COUNT
} menu_state_t;

typedef enum {
    BTN_UP,
    BTN_DOWN,
    BTN_SELECT,
    BTN_ESC,
    BTN_COUNT,
    BTN_NONE
} button_t;

typedef struct {
    uint8_t var;
    const uint8_t min;
    const uint8_t max;
} option_item_t;

typedef struct {
    option_item_t *option_tbl_ptr;
    uint8_t option_items;
    const char *text;
    char separator;
} option_t;

typedef struct {
    const char *text;
    void (*menu_handler)(void);
} menu_item_t;

typedef struct {
    void (*handler)(void);
    menu_state_t next_state;
} fsm_entry_t;

//---Globals---
static menu_state_t state = STATE_MENU_NAV;
static uint8_t v_index = 0;
static uint8_t h_index = 0;
static option_t active_options;

//---Function Prototypes---
static void Clock(void);
static void Date(void);
static void ExitMenu(void);

//---Menu---
static const menu_item_t menu[]={
    {"Set Clock",&Clock},
    {"Set Date",&Date},
    {"Exit",&ExitMenu}
};
#define MENU_SIZE (sizeof(menu)/sizeof(menu[0]))

//---Clock Options---
static option_item_t clock_options[] = {
    {12,0,23}, //hours
    {0,0,59},  //minutes
    {0,0,59}   //seconds
};

//---Date Options---
static option_item_t date_options[] = {
    {1,1,31},   //day
    {1,1,12},   //month
    {25,00,99}  //year
};

//---VIEW FUNCTIONS---
static void ShowMenu(uint8_t index) {
    printf("\033[H\033[J");//clear screen
    printf("\033[?25l");//disable cursor
    printf("\n=== MENU ===\n");
    static const menu_item_t *menu_ptr = menu;

    for (uint8_t i = 0; i < MENU_SIZE; i++) {
        if (i == index) printf(" > %s\n", menu_ptr[i].text);
        else printf("   %s\n", menu_ptr[i].text);
    }
}

static void ShowOptions(uint8_t index) {
    printf("\033[H\033[J");
    printf("\n=== OPTIONS ===\n");
    
    printf("%s", active_options.text);
    for (uint8_t i = 0; i < active_options.option_items; i++) {
        printf("%02hhu", active_options.option_tbl_ptr[i].var);
        if (i < active_options.option_items - 1) printf("%c", active_options.separator);
    }
    printf("\n%*s--\n", (int) index * 3 + 6, "");
}

static void EditOptions(uint8_t index) {
    printf("\033[H\033[J");//clear screen
    printf("\n=== OPTIONS ===\n");
    
    printf("%s", active_options.text);
    for (uint8_t i = 0; i < active_options.option_items; i++) {
        printf("%02hhu", active_options.option_tbl_ptr[i].var);
        if (i < active_options.option_items - 1) printf("%c", active_options.separator);
    }
    printf("\n%*s[%02hhu]\n", (int) index * 3 + 5, "", active_options.option_tbl_ptr[index].var);
}

//---Handlers---
static void Menu_Move_Up(void) {
    if (v_index > 0) v_index--;
    ShowMenu(v_index);
}
static void Menu_Move_Down(void) {
    if (v_index < MENU_SIZE-1) v_index++;
    ShowMenu(v_index);
}
static void Menu_Select(void) {
    menu[v_index].menu_handler();
}
static void Menu_Esc(void) {
    printf("Exiting...\n");
    exit(EXIT_SUCCESS);
}

static void Option_Move_Left(void) {
    if (h_index > 0) h_index--;
    ShowOptions(h_index);
}
static void Option_Move_Right(void) {
    if (h_index < active_options.option_items-1) h_index++;
    ShowOptions(h_index);
}
static void Option_Select(void) {
    EditOptions(h_index);
}
static void Option_Esc(void) {
    ShowMenu(v_index);
}

static void Edit_Add(void) {
    option_item_t *item = &active_options.option_tbl_ptr[h_index];
    if (item->var < item->max) item->var++;
    EditOptions(h_index);
}
static void Edit_Subtract(void) {
    option_item_t *item = &active_options.option_tbl_ptr[h_index];
    if (item->var > item->min) item->var--;
    EditOptions(h_index);
}
static void Edit_Select(void) {
    ShowOptions(h_index);
}
static void Edit_Esc(void) {
    ShowOptions(h_index);
}

//--- MENU HANDLERS ---
static void Clock(void) {
    active_options.option_tbl_ptr = clock_options;
    active_options.option_items = sizeof(clock_options)/sizeof(clock_options[0]);
    active_options.text = "Time: ";
    active_options.separator = ':';
    h_index = 0;
    ShowOptions(0);
}
static void Date(void) {
    active_options.option_tbl_ptr = date_options;
    active_options.option_items = sizeof(date_options)/sizeof(date_options[0]);
    active_options.text = "Date: ";
    active_options.separator = '/';
    h_index = 0;
    ShowOptions(0);
}
static void ExitMenu(void) {
    printf("Exiting...\n");
    exit(EXIT_SUCCESS);
}

static const fsm_entry_t fsm_table[STATE_COUNT][BTN_COUNT] = {
    {//STATE_MENU_NAV
        {Menu_Move_Up,      STATE_MENU_NAV},    //BTN_UP
        {Menu_Move_Down,    STATE_MENU_NAV},    //BTN_DOWN
        {Menu_Select,       STATE_OPTION_NAV},  //BTN_SELECT
        {Menu_Esc,          STATE_MENU_NAV}     //BTN_ESC
    },
    {//STATE_OPTION_NAV
        {Option_Move_Left,  STATE_OPTION_NAV},  //BTN_UP
        {Option_Move_Right, STATE_OPTION_NAV},  //BTN_DOWN
        {Option_Select,     STATE_OPTION_EDIT}, //BTN_SELECT
        {Option_Esc,        STATE_MENU_NAV}     //BTN_ESC
    },
    {//STATE_OPTION_EDIT
        {Edit_Add,          STATE_OPTION_EDIT}, //BTN_UP
        {Edit_Subtract,     STATE_OPTION_EDIT}, //BTN_DOWN
        {Edit_Select,       STATE_OPTION_NAV},  //BTN_SELECT
        {Edit_Esc,          STATE_OPTION_NAV}   //BTN_ESC
    }
};

//--- INPUT HANDLER ---
static button_t GetButton(void) {
    char ch = getchar();
    ch = toupper(ch);
    switch (ch) {
        case 'Q': return BTN_UP;    //Up
        case 'W': return BTN_DOWN;  //Down
        case 'E': return BTN_SELECT;//Select
        case 'R': return BTN_ESC;   //Esc
        default: return BTN_NONE;
    }
}

//---FSM---
void ARRAY_OF_STRUCTS_Run_FSM(void) {
    ShowMenu(0);
    while (1) {
        button_t btn = GetButton();
        if (btn == BTN_NONE) continue;
        fsm_entry_t entry = fsm_table[state][btn];
        if (entry.handler) entry.handler();
        state = entry.next_state;
    }
}
