//
// Created by Brandon on 8/17/2015.
//

#include "lc3.h"

void lc3_init(lc3 *ptr)
{
    for(int i = 0; i < GPR_NUM; i++)
    {
        ptr->registers[i] = 0x0000;
    }
    ptr->pc = 0;
    ptr->psr= 0x2;
    ptr->mar= 0;
    ptr->mdr= 0;
    ptr->disk = NULL;
    ptr->diskStatus = 0;
    ptr->mem[0xFFFE] = 0xFFFF;
}


enum CycleReturn lc3_cycle(lc3 *ptr)
{
    if(!lc3_checkMachine(ptr))
        return HALT;

                                    //FETCH
    ptr->mar = ptr->pc;
    ptr->pc++;                      //Increment program counter
    if(lc3_checkInterrupts(ptr))
        return INTERRUPT;

    ptr->mdr = ptr->mem[ptr->mar];   //Load the instruction
    ptr->ir = ptr->mdr;

                                            //DECODE
    uint16_t opcode = ((ptr->ir)>>12)&0xF; //Shift the opcode to get the bits
                                            //and EXECUTE
    switch(opcode)
    {
        case OP_ADD:
            lc3_ANDD(ptr);
            break;
        case OP_AND:
            lc3_ANDD(ptr);
            break;
        case OP_BR:
            lc3_BR(ptr);
            break;
        case OP_JMP:
            lc3_JMP(ptr);
            break;
        case OP_JSR:
            lc3_JSR(ptr);
            break;
        case OP_LD:
            lc3_LS(ptr);
            break;
        case OP_LDI:
            lc3_LSI(ptr);
            break;
        case OP_LDR:
            lc3_LSR(ptr);
            break;
        case OP_LEA:
            lc3_LEA(ptr);
            break;
        case OP_NOT:
            lc3_NOT(ptr);
            break;
        case OP_RESV:
            lc3_RESV(ptr);
            break;
        case OP_RTI:
            lc3_RTI(ptr);
            break;
        case OP_ST:
            lc3_LS(ptr);
            break;
        case OP_STI:
            lc3_LSI(ptr);
            break;
        case OP_STR:
            lc3_LSR(ptr);
            break;
        case OP_TRAP:
            lc3_TRAP(ptr);
            break;
    }

    lc3_ioHandle(ptr);

    return SUCCESS;
}

uint8_t lc3_checkInterrupts(lc3 *ptr)
{
    //KBDR check
    //Check KBDR interrupt bit
    if(ptr->mem[KBSR] & 0x4000)
    {
        //Check if the int line has been triggered
        if(lc3_getIntLine(ptr, IV_KB))
        {
            lc3_interrupt(ptr, IV_KB, 4);
            lc3_clearIntLine(ptr, IV_KB);
            return 1;
        }
    }

    return 0;
}

void lc3_ioHandle(lc3 *ptr)
{

}

void lc3_keyHandle(lc3 *ptr, uint8_t key)
{
    if(! (ptr->mem[KBSR] & 0x8000))
    {
        ptr->mem[KBDR] = key & 0xFF;      //Set the character
        ptr->mem[KBSR] |= 0x8000;         //Set the freshness bit
        if(ptr->mem[KBSR] & 0x4000)
        {
            lc3_setIntLine(ptr, IV_KB);
        }
    }
}

inline int16_t lc3_SEXT(uint16_t input, uint8_t bits)
{
    int i = 0;
    uint8_t numshift = 16 - bits;
    int16_t num = (int16_t) input;
    for(i = 0; i<numshift; i++)
    {
        num<<=1;
    }

    return (num)>>numshift;
}

inline void lc3_setcc(lc3 *ptr, uint16_t num)
{
    if(num == 0)
    {
        ptr->psr = (ptr->psr & ~0x7)|0x2;
    }
    else if (num & 0x8000)
    {
        ptr->psr = (ptr->psr & ~0x7)|0x4;
    }
    else
    {
        ptr->psr = (ptr->psr & ~0x7)|0x1;
    }
}

inline uint8_t lc3_getIntLine(lc3 *ptr, uint8_t intLine)
{
    uint8_t output;
    output = ((ptr->intLines[intLine/8])>>(intLine%8))&0x1;
    return output;
}
inline void lc3_setIntLine(lc3 *ptr, uint8_t intLine)
{
    ptr->intLines[intLine/8] |= (0x1)<<(intLine%8);
}
inline void lc3_clearIntLine(lc3 *ptr, uint8_t intLine)
{
    ptr->intLines[intLine/8] &= ~((0x1)<<(intLine%8));
}
inline uint8_t lc3_checkMachine(lc3 *ptr)
{
    return (ptr->mem[0xFFFE] & 0x8000)>>15;
}

void lc3_interrupt(lc3 *ptr, uint8_t intNum, uint8_t priority)
{
    uint8_t currpriority = (ptr->psr >> 8)&0x7;
    if(priority > currpriority)
    {
        //If priority is higher than current priority, do it

        //Push the PSR and PC
        //Quick version
        ptr->registers[6]--;
        ptr->mem[ptr->registers[6]] = ptr->psr;
        ptr->registers[6]--;
        ptr->mem[ptr->registers[6]] = ptr->pc-1;    //-1 because pc already incremented

        ptr->psr &= 0x7FFF; //Clear the mode bit, so PSR[15] is now 0
        ptr->psr = (ptr->psr & ~0x700) | ((priority&0x7)<<8);   //Set the priority
        ptr->mar = 0x100 + intNum;
        ptr->mdr = ptr->mem[ptr->mar];
        ptr->mar = ptr->mdr;
        ptr->pc = ptr->mdr;                                     //PC of interrupt SR

    }

}

//Loading functions

//The classic loading scheme we all know and love
/*
void lc3_loadObj(lc3 *ptr, obj_file obj)
{
    size_t i = 0;
    uint16_t entry = obj.buffer[i]; //Set the pc
    ptr->pc = entry;
    uint16_t* mem = &(ptr->mem[0]);  //Have a quick to access pointer to memory
    i++;
    for(; i < obj.size; i++)
    {
        if(entry+i > 0x10000)
            return;
        mem[entry+i-1] = obj.buffer[i];
    }
}
*/

//MDR still has the instruction
inline void lc3_BR(lc3 *ptr)
{
    uint16_t cc = (ptr->psr)&0x7;   //Mask the cc bits
    uint16_t con = ((ptr->ir)>>9)&0x7; //Shift and mask the condition branch
    if(((cc&0x4) & (con&0x4)) | ((cc&0x2) & (con&0x2)) | ((cc&0x1) & (con&0x1)))
    {
        ptr->pc = ptr->pc + lc3_SEXT((ptr->ir) & 0x1FF, 9);
    }
}
inline void lc3_ANDD(lc3 *ptr)
{
    uint16_t inst = ptr->ir;
    int16_t src1;
    int16_t src2;
    int16_t res;
    src1 = ptr->registers[(inst>>6)&0x7];   //src1 is a register. Always
    if(inst & 0x0020)
    {
        //Immediate
        src2 = lc3_SEXT(inst & 0x1F, 5);
    }
    else
    {
        //Register
        src2 = ptr->registers[(inst)&0x7];
    }
    if(inst & 0x4000)
        res = src1 & src2;
    else
        res = src1 + src2;
    ptr->registers[(inst>>9)&0x7] = res;
    lc3_setcc(ptr, (uint16_t) res);
}
inline void lc3_LS(lc3 *ptr)
{
    uint16_t inst = ptr->ir;
    ptr->mar = ptr->pc + lc3_SEXT((inst) & 0x1FF, 9);
    uint16_t* regptr = &(ptr->registers[(inst>>9)&0x7]);
    lc3_MEM(ptr,inst,regptr);
}
inline void lc3_JSR(lc3 *ptr)
{
    uint16_t inst = ptr->ir;
    ptr->registers[7] = ptr->pc;    //Save PC
    if(inst & 0x0800)
    {
        //JSR
        ptr->pc = ptr->pc + lc3_SEXT(inst & 0x07FF, 11);
    }
    else
    {
        //JSRR
        ptr->pc = ptr->registers[(inst >> 6)&0x7];
    }
}
inline void lc3_LSR(lc3 *ptr)
{
    uint16_t inst = ptr->ir;
    ptr->mar = ptr->registers[(inst>>6)&0x7] + lc3_SEXT(inst & 0x3F, 6);
    uint16_t* regptr = &(ptr->registers[(inst>>9)&0x7]);
    lc3_MEM(ptr,inst,regptr);
}
inline void lc3_RTI(lc3 *ptr)
{
    if(ptr->psr & 0x8000)
    {
        //Exception!
    }
    else
    {
        //RTI
        ptr->pc = ptr->mem[ptr->registers[6]];   //Pop off the pc
        (ptr->registers[6])++;
        ptr->psr = ptr->mem[ptr->registers[6]];  //Pop off the psr
        (ptr->registers[6])++;
    }
}
inline void lc3_NOT(lc3 *ptr)
{
    uint16_t inst = ptr->ir;
    uint16_t* drptr = &(ptr->registers[(inst>>9)&0x7]);
    uint16_t* srptr = &(ptr->registers[(inst>>6)&0x7]);
    *drptr = ~(*srptr);
    lc3_setcc(ptr, *drptr);
}
inline void lc3_LSI(lc3 *ptr)
{
    uint16_t inst = ptr->ir;
    ptr->mar = ptr->pc + lc3_SEXT(inst & 0x1FF, 9);
    ptr->mdr = ptr->mem[ptr->mar];
    ptr->mar = ptr->mdr;
    uint16_t* regptr = &(ptr->registers[(inst>>9)&0x7]);
    lc3_MEM(ptr,inst,regptr);
}
inline void lc3_JMP(lc3 *ptr)
{
    ptr->pc = ptr->registers[((ptr->ir)>>6)&0x7];
}
inline void lc3_RESV(lc3 *ptr)
{

}
inline void lc3_LEA(lc3 *ptr)
{
    uint16_t* regptr = &(ptr->registers[((ptr->ir) >>9)&0x7]);
    *regptr = ptr->pc + lc3_SEXT(ptr->ir & 0x1FF, 9);
    lc3_setcc(ptr, *regptr);
}
inline void lc3_TRAP(lc3 *ptr)
{
    //Hacked in halt
    if((ptr->ir & 0xFF) == 0x25)
    {
        ptr->mem[0xFFFE] = ptr->mem[0xFFFE] & 0x7FFF; //Clear it
        return;
    }
    ptr->mar = ptr->ir & 0xFF;
    ptr->mdr = ptr->mem[ptr->mar];
    ptr->registers[7] = ptr->pc;
    ptr->pc = ptr->mdr;


}
inline void lc3_MEM(lc3 *ptr, uint16_t inst, uint16_t *regptr)
{
    if(inst & 0x1000)
    {
        //S
        ptr->mdr = *regptr;
        ptr->mem[ptr->mar] = ptr->mdr;
        if(ptr->mar == DDR)
        {
            //If the DDR is written to, clear the freshness flag
            ptr->mem[DSR] *= 0x7FFF;
        }
    }
    else
    {
        //L
        if(ptr->mar == KBDR)
        {
            //If the KBDR is read, clear the freshness flag
            ptr->mem[KBSR] &= 0x7FFF;
        }
        ptr->mdr = ptr->mem[ptr->mar];
        *regptr = ptr->mdr;
        lc3_setcc(ptr, *regptr);
    }
}