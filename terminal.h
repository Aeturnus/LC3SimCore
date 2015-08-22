//
// Created by Brandon on 8/22/2015.
//
// This provides the backend implementation of a 80x24 terminal. It should have no idea what the LC3 is, but will set the interrupt line accordingly

#ifndef LC3SIMCORE_TERMINAL_H
#define LC3SIMCORE_TERMINAL_H

#include <stdint.h>

#define T_COLS  80
#define T_ROWS  24
#define T_SIZE T_COLS*T_ROWS

typedef struct term_str

{
    uint8_t buffer[T_SIZE];        //The buffer that holds all the character data
    uint8_t r;              //row
    uint8_t c;              //column
    uint8_t* intLine_ptr;   //Pointer to the cluster interrupt line for the terminal
    uint8_t intLine_shift;  //Bit number for the particular line
    uint16_t* kbdr_ptr;     //Pointer to the KBDR in memory.
    uint16_t* kbsr_ptr;     //Pointer to the KBSR in memory.
    uint16_t* ddr_ptr;      //Pointer to the DDR in memory.
    uint16_t* dsr_ptr;      //Pointer to the DDR in memory.
}term;

void term_init(term* ptr, uint8_t* intLine, uint8_t intLine_shift, uint16_t* kbdr, uint16_t* kbsr, uint16_t* ddr, uint16_t* dsr);
uint8_t term_getChar(term* ptr,uint8_t input);
uint8_t term_printChar(term* ptr);
void term_shiftUp(term* ptr);

#endif //LC3SIMCORE_TERMINAL_H
