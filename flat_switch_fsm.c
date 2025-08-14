#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>

//--Custom Types--
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
    BTN_NONE
} button_t;

typedef struct {
    const unsigned char* text;
    void (*menu_handler)(void);
} menu_item_t;
#define MENU_SIZE (sizeof(menu)/sizeof(menu[0]))

typedef struct {
    uint8_t var;
    const uint8_t min;
    const uint8_t max;
} option_item_t;

typedef struct {
    option_item_t *option_tbl_ptr;
    uint8_t option_items;
    const char* text;
    char separator;
} option_t;

//---Globals ---
static menu_state_t State = STATE_MENU_NAV;
static uint8_t v_index = 0;
static uint8_t h_index = 0;
static option_t active_options;

//--Function Prototypes--
static void Clock(void);
static void Date(void);
static void ExitMenu(void);
static void ShowMenu(uint8_t index);
static void ShowOptions(uint8_t index);
static void EditOptions(uint8_t index);

//--Menu--
static const menu_item_t menu[] = {
    {"Set Clock", &Clock},
    {"Set Date", &Date},
    {"Exit", &ExitMenu}
};

//--Clock Options--
static option_item_t clock_options[] = {
    {12, 0, 23}, //hours
    {0, 0, 59}, //minutes
    {0, 0, 59} //seconds
};

//--Date Options--
static option_item_t date_options[] = {
    {1, 1, 31}, //day
    {1, 1, 12}, //month
    {25, 0, 99} //year
};


//--- STATE HANDLERS ---
static void HandleMenuNav(button_t btn) {
    switch (btn) {
        case BTN_UP:
            if (v_index > 0) v_index--;
            ShowMenu(v_index);
            break;
        case BTN_DOWN:
            if (v_index < MENU_SIZE - 1) v_index++;
            ShowMenu(v_index);
            break;
        case BTN_SELECT:
            menu[v_index].menu_handler();
            State = STATE_OPTION_NAV;
            break;
        case BTN_ESC:
            printf("Exiting...\n");
            exit(EXIT_SUCCESS);
        default:
            break;
    }
}

static void HandleOptionNav(button_t btn) {
    switch (btn) {
        case BTN_UP:
            if (h_index > 0) h_index--;
            ShowOptions(h_index);
            break;
        case BTN_DOWN:
            if (h_index < active_options.option_items - 1) h_index++;
            ShowOptions(h_index);
            break;
        case BTN_SELECT:
            State = STATE_OPTION_EDIT;
            EditOptions(h_index);
            break;
        case BTN_ESC:
            State = STATE_MENU_NAV;
            ShowMenu(v_index);
            break;
        default:
            break;
    }
}

static void HandleOptionEdit(button_t btn) {
    switch (btn) {
        case BTN_UP:
            if (active_options.option_tbl_ptr[h_index].var < active_options.option_tbl_ptr[h_index].max) {
                active_options.option_tbl_ptr[h_index].var++;
            }
            EditOptions(h_index);
            break;
        case BTN_DOWN:
            if (active_options.option_tbl_ptr[h_index].var > active_options.option_tbl_ptr[h_index].min) {
                active_options.option_tbl_ptr[h_index].var--;
            }
            EditOptions(h_index);
            break;
        case BTN_SELECT: // Do nothing
            State = STATE_OPTION_NAV;
            ShowOptions(h_index);
            break;
        case BTN_ESC: // Return        
            State = STATE_OPTION_NAV;
            ShowOptions(h_index);
            break;
        default:
            break;
    }
}


//---VIEW FUNCTIONS ---
static void ShowMenu(uint8_t index) {
    printf("\033[H\033[J");//clear screen
    printf("\033[?25l");//disable cursor
    printf("\n=== MENU ===\n");
    static const menu_item_t *menu_ptr = menu;

    for (uint8_t i = 0; i < MENU_SIZE; i++) {
        if (i == index) {
            printf(" > %s\n", menu_ptr[i].text);
        } else {
            printf("   %s\n", menu_ptr[i].text);
        }
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

//--- MENU HANDLERS ---//
static void Clock(void) {
    active_options.option_tbl_ptr = clock_options;
    active_options.option_items = sizeof (clock_options) / sizeof (clock_options[0]);
    active_options.text = "Time: ";
    active_options.separator = ':';
    h_index = 0;
    ShowOptions(h_index);
}

static void Date(void) {
    active_options.option_tbl_ptr = date_options;
    active_options.option_items = sizeof (date_options) / sizeof (date_options[0]);
    active_options.text = "Date: ";
    active_options.separator = '/';
    h_index = 0;
    ShowOptions(h_index);
}

static void ExitMenu(void) {
    printf("Exiting...\n");
    exit(EXIT_SUCCESS);
}

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
void FLAT_SWITCH_Run_FSM(void) {
    ShowMenu(v_index);

    while (1) {
        button_t btn = GetButton();
        if (btn == BTN_NONE) continue;

        switch (State) {
            case STATE_MENU_NAV:
                HandleMenuNav(btn);
                break;
            case STATE_OPTION_NAV:
                HandleOptionNav(btn);
                break;
            case STATE_OPTION_EDIT:
                HandleOptionEdit(btn);
                break;
        }
    }
}