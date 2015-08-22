//
// Created by Brandon on 8/22/2015.
//

#include "terminal.h"

void term_init(term* ptr, uint8_t* intLine, uint8_t intLine_shift, uint16_t* kbdr, uint16_t* kbsr, uint16_t* ddr, uint16_t* dsr)
{
    //Clear buffer
    for(int i = 0; i < T_SIZE; i++)
    {
        ptr->buffer[i] = 0;
    }
    ptr->intLine_ptr = intLine;
    ptr->intLine_shift = intLine_shift;
    ptr->kbdr_ptr = kbdr;
    ptr->kbsr_ptr = kbsr;
    ptr->ddr_ptr = ddr;
    ptr->dsr_ptr = dsr;
}
uint8_t term_getChar(term* ptr,uint8_t input)
{
    //Check if freshness flag is cleared
    if((*(ptr->kbsr_ptr) & 0x8000) == 0)
    {
        //If it is, proceed
        *(uint8_t*)(ptr->kbsr_ptr) = input;
        //Set int line if interrupts are enabled
        if(*(ptr->kbsr_ptr) & 0x4000)
        {
            *(ptr->intLine_ptr) |= ((0x1)<<(ptr->intLine_shift));
        }
    }
    else
    {
        return 0;
    }
    return 1;
}
uint8_t term_printChar(term* ptr)
{
    //Check if the freshness flag is cleared: if it's set, it's looking for another character
    if((*(ptr->dsr_ptr) & 0x8000) == 0)
    {
        uint8_t col = ptr->c;
        uint8_t row = ptr->r;
        ptr->buffer[T_COLS * row + col] = *((uint8_t*)(ptr->ddr_ptr));
        col++;
        if(col>= 80)
        {
            row++;
        }
        if(row >= 24)
        {
            col = 0;
            row = 23;
            term_shiftUp(ptr);
        }
    }
    else
    {
        return 0;
    }
    return 1;
}
void term_shiftUp(term* ptr)
{
    uint8_t* buffer = ptr->buffer;
    for(int i = T_COLS; i < (T_SIZE-T_COLS); i++)
    {
        buffer[i] = buffer[i + T_COLS];
    }
    for(int i = T_SIZE-T_COLS; i < T_SIZE; i++)
    {
        buffer[i] = 0;
    }
}
