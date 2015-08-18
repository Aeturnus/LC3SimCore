//
// Created by Brandon on 8/17/2015.
//

#include "LC3.h"

void LC3_init(LC3* ptr)
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
}


enum CycleReturn LC3_cycle(LC3* ptr)
{
    if(!(ptr->memory.mem[0xFFFE] & 0x8000))
        return HALT;
                                            //FETCH
    ptr->mdr = ptr->memory.mem[ptr->pc];    //Load the instruction
    ptr->ir = ptr->mdr;
    ptr->pc++;                              //Increment program counter

                                            //DECODE
    uint16_t opcode = ((ptr->ir)>>12)&0xF; //Shift the opcode to get the bits
                                            //and EXECUTE
    switch(opcode)
    {
        case OP_ADD:
            LC3_ANDD(ptr);
            break;
        case OP_AND:
            LC3_ANDD(ptr);
            break;
        case OP_BR:
            LC3_BR(ptr);
            break;
        case OP_JMP:
            LC3_JMP(ptr);
            break;
        case OP_JSR:
            LC3_JSR(ptr);
            break;
        case OP_LD:
            LC3_LS(ptr);
            break;
        case OP_LDI:
            LC3_LSI(ptr);
            break;
        case OP_LDR:
            LC3_LSR(ptr);
            break;
        case OP_LEA:
            LC3_LEA(ptr);
            break;
        case OP_NOT:
            LC3_NOT(ptr);
            break;
        case OP_RESV:
            LC3_RESV(ptr);
            break;
        case OP_RTI:
            LC3_RTI(ptr);
            break;
        case OP_ST:
            LC3_LS(ptr);
            break;
        case OP_STI:
            LC3_LSI(ptr);
            break;
        case OP_STR:
            LC3_LSR(ptr);
            break;
        case OP_TRAP:
            LC3_TRAP(ptr);
            break;
    }

    //LC3_ioHandle(ptr);

    return SUCCESS;
}

void LC3_ioHandle(LC3* ptr)
{

}

inline int16_t LC3_SEXT(uint16_t input,uint8_t bits)
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

inline void LC3_setcc(LC3* ptr, uint16_t num)
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


//Loading functions

//The classic loading scheme we all know and love
void LC3_loadObj(LC3* ptr, obj_file obj)
{
    size_t i = 0;
    uint16_t entry = obj.buffer[i]; //Set the pc
    ptr->pc = entry;
    uint16_t* mem = &(ptr->memory.mem[0]);  //Have a quick to access pointer to memory
    i++;
    for(; i < obj.size; i++)
    {
        if(entry+i > 0x10000)
            return;
        mem[entry+i-1] = obj.buffer[i];
    }
}

//MDR still has the instruction
inline void LC3_BR(LC3* ptr)
{
    uint16_t cc = (ptr->psr)&0x7;   //Mask the cc bits
    uint16_t con = ((ptr->ir)>>9)&0x7; //Shift and mask the condition branch
    if(((cc&0x4) & (con&0x4)) | ((cc&0x2) & (con&0x2)) | ((cc&0x1) & (con&0x1)))
    {
        ptr->pc = ptr->pc + LC3_SEXT((ptr->ir)&0x1FF,9);
    }
}
inline void LC3_ANDD(LC3* ptr)
{
    uint16_t inst = ptr->ir;
    int16_t src1;
    int16_t src2;
    int16_t res;
    src1 = ptr->registers[(inst>>6)&0x7];   //src1 is a register. Always
    if(inst & 0x0020)
    {
        //Immediate
        src2 = LC3_SEXT(inst & 0x1F,5);
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
    LC3_setcc(ptr,(uint16_t)res);
}
inline void LC3_LS(LC3* ptr)
{
    uint16_t inst = ptr->ir;
    ptr->mar = ptr->pc + LC3_SEXT((inst)&0x1FF,9);
    uint16_t* regptr = &(ptr->registers[(inst>>9)&0x7]);
    if(inst & 0x1000)
    {
        //ST
        ptr->mdr = *regptr;
        ptr->memory.mem[ptr->mar] = ptr->mdr;
    }
    else
    {
        //LD
        ptr->mdr = ptr->memory.mem[ptr->mar];
        *regptr = ptr->mdr;
        LC3_setcc(ptr,*regptr);
    }
}
inline void LC3_JSR(LC3* ptr)
{
    uint16_t inst = ptr->ir;
    ptr->registers[7] = ptr->pc;    //Save PC
    if(inst & 0x0800)
    {
        //JSR
        ptr->pc = ptr->pc + LC3_SEXT(inst & 0x07FF,11);
    }
    else
    {
        //JSRR
        ptr->pc = ptr->registers[(inst >> 6)&0x7];
    }
}
inline void LC3_LSR(LC3* ptr)
{
    uint16_t inst = ptr->ir;
    ptr->mar = ptr->registers[(inst>>6)&0x7] + LC3_SEXT(inst&0x3F,6);
    uint16_t* regptr = &(ptr->registers[(inst>>9)&0x7]);
    if(inst & 0x1000)
    {
        //STR
        ptr->mdr = *regptr;
        ptr->memory.mem[ptr->mar] = ptr->mdr;
    }
    else
    {
        //LDR
        ptr->mdr = ptr->memory.mem[ptr->mar];
        *regptr = ptr->mdr;
        LC3_setcc(ptr,*regptr);
    }
}
inline void LC3_RTI(LC3* ptr)
{
    if(ptr->psr & 0x8000)
    {
        //Exception!
    }
    else
    {
        //RTI
        ptr->pc = ptr->memory.mem[ptr->registers[6]];   //Pop off the pc
        (ptr->registers[6])++;
        ptr->psr = ptr->memory.mem[ptr->registers[6]];  //Pop off the psr
        (ptr->registers[6])++;
    }
}
inline void LC3_NOT(LC3* ptr)
{
    uint16_t inst = ptr->ir;
    uint16_t* drptr = &(ptr->registers[(inst>>9)&0x7]);
    uint16_t* srptr = &(ptr->registers[(inst>>6)&0x7]);
    *drptr = ~(*srptr);
    LC3_setcc(ptr,*drptr);
}
inline void LC3_LSI(LC3* ptr)
{
    uint16_t inst = ptr->ir;
    ptr->mar = ptr->pc + LC3_SEXT(inst & 0x1FF,9);
    ptr->mdr = ptr->memory.mem[ptr->mar];
    ptr->mar = ptr->mdr;
    uint16_t* regptr = &(ptr->registers[(inst>>9)&0x7]);
    if(inst & 0x1000)
    {
        //STI
        ptr->mdr = *regptr;
        ptr->memory.mem[ptr->mar] = ptr->mdr;
    }
    else
    {
        //LDI
        ptr->mdr = ptr->memory.mem[ptr->mar];
        *regptr = ptr->mdr;
        LC3_setcc(ptr,*regptr);
    }
}
inline void LC3_JMP(LC3* ptr)
{
    ptr->pc = ptr->registers[((ptr->ir)>>6)&0x7];
}
inline void LC3_RESV(LC3* ptr)
{

}
inline void LC3_LEA(LC3* ptr)
{
    uint16_t* regptr = &(ptr->registers[((ptr->ir) >>9)&0x7]);
    *regptr = ptr->pc + LC3_SEXT(ptr->ir & 0x1FF,9);
    LC3_setcc(ptr,*regptr);
}
inline void LC3_TRAP(LC3* ptr)
{
    //Hacked in halt
    if((ptr->ir & 0xFF) == 0x25)
    {
        ptr->memory.mem[0xFFFE] = ptr->memory.mem[0xFFFE] & 0x7FFF; //Clear it
        return;
    }
    ptr->registers[7] = ptr->pc;
    ptr->pc = ptr->memory.mem[ptr->ir & 0xFF];
}

