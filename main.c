/* 
 * Three FSM Implementations
 * 1) Nested Switch
 * 2) Flat Switch with independent handlers
 * 3) Table Based with a 2D array of structs
 * 
 * Author: Panagiotis Argyropoulos - pargyropoulos@gmail.com
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "fsm.h"

int main(void) {

    NESTED_SWITCH_Run_FSM();
//    FLAT_SWITCH_Run_FSM();
//    ARRAY_OF_STRUCTS_Run_FSM();
}

