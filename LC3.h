//
// Created by Brandon on 8/17/2015.
//

#ifndef LC3SIMCORE_LC3_H
#define LC3SIMCORE_LC3_H

#include <stdio.h>
#include <stdint.h>



#define GPR_NUM 8

#define OP_BR   0x0
#define OP_ADD  0x1
#define OP_LD   0x2
#define OP_ST   0x3
#define OP_JSR  0x4
#define OP_AND  0x5
#define OP_LDR  0x6
#define OP_STR  0x7
#define OP_RTI  0x8
#define OP_NOT  0x9
#define OP_LDI  0xA
#define OP_STI  0xB
#define OP_JMP  0xC
#define OP_RESV 0xD
#define OP_LEA  0xE
#define OP_TRAP 0xF

//Memory mapped io addrs
//80x24 terminal
#define KBSR    0xFE00
#define KBDR    0xFE02
#define DSR     0xFE04
#define DDR     0xFE06
#define MCR     0xFFFE

//New io device:
#define HDSR    0xFE08  //Hard disk status register
#define HDDR    0xFE0A  //Hard disk data register
struct Memory_str
{
    uint16_t mem[0x10000];  //All the memory
};

typedef struct LC3_str
{
    uint16_t registers[8];  //General purpose regs
    uint16_t psr;           //process status register
    uint16_t pc;            //program counter
    uint16_t ir;           //Memory address register
    uint16_t mar;           //Memory address register
    uint16_t mdr;           //Memory data register
    struct Memory_str memory;      //Memory struct
    uint8_t intLines[32];   //A bit for each possible interrupt
    uint8_t* disk;          //Attach a disk to the LC3 when wanted
    uint8_t diskStatus;     //How many cycles before disk read is complete
    uint8_t vram[1920];     //Memory for the text display. Not accessible to user.
}LC3;

typedef struct obj_str
{
    size_t size;
    uint16_t* buffer;
    uint8_t allocated;
}obj_file;

enum CycleReturn
{
    SUCCESS,
    HALT,
    EXCEPTION
};

void LC3_init(LC3* ptr);
enum CycleReturn LC3_cycle(LC3* ptr);
void LC3_ioHandle(LC3* ptr);
uint8_t LC3_checkIntLine(LC3*ptr, uint8_t vector);

//IO handlers
void LC3_keyHandle(LC3* ptr, uint8_t key);      //Function to deal with keyboard
void LC3_disHandle(LC3* ptr);
void LC3_diskHandle(LC3* ptr);

//Helper functions
int16_t LC3_SEXT(uint16_t input,uint8_t bitnum);
void LC3_setcc(LC3* ptr, uint16_t num);

//Classic loading scheme: first word is entry point
void LC3_loadObj(LC3* ptr,obj_file obj);

//Inline functions for each instruction
void LC3_BR(LC3* ptr);
void LC3_ANDD(LC3* ptr);
void LC3_LS(LC3* ptr);
void LC3_JSR(LC3* ptr);
void LC3_LSR(LC3* ptr);
void LC3_RTI(LC3* ptr);
void LC3_NOT(LC3* ptr);
void LC3_LSI(LC3* ptr);
void LC3_JMP(LC3* ptr);
void LC3_RESV(LC3* ptr);
void LC3_LEA(LC3* ptr);
void LC3_TRAP(LC3* ptr);


#endif //LC3SIMCORE_LC3_H
