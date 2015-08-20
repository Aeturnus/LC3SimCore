//
// Created by Brandon on 8/19/2015.
//

#ifndef LC3SIMCORE_DEBUGGER_H
#define LC3SIMCORE_DEBUGGER_H

#include <stdint.h>

#include "lc3.h"

enum FileStatus
{
    OPEN,
    NOTVALID,
    PARITY
};


//Conditional types
#define bp_eq   0
#define bp_g    1
#define bp_l    2
#define bp_geq  3
#define bp_leq  4
//Source type
#define bp_reg  0
#define bp_mem  1
//Breakpoint struct
typedef struct bp_str
{
    uint8_t isSet;      //0: not set/1: set
    uint8_t isCon;      //0: unconditional/ 1: conditional
    uint8_t conType;    //Type of conditional
    uint8_t srcType;    //Type of source to check: mem or reg
    uint16_t srcNum;    //How it's addressed: reg 0-7 or mem 0x0000 - 0xFFFF
    int16_t testVal;    //Test value (signed)

}bp;

//Debugger struct
typedef struct db_str
{
    struct lc3_str *lc3ptr;
    struct bp_str breakpoints[0x10000];  //1:1 mapping of mem locs to breakpoints
}db;

enum FileStatus db_openObj(char* filePath,obj_file* obj);
void db_deleteObj(obj_file* obj);

//"step" will be a single cycle run. Others will call this
void db_step(db* ptr);
//"stepOver" will be a single cycle run unless it's a JSR/JSRR call that will continue running until PC equals the next instruction
void db_stepOver(db* ptr);
//"stepOut" will run until JMP R7 is hit
void db_stepOut(db* ptr);
//"run" will continue until a breakpoint is hit
void db_run(db* ptr);

//This will be run at the end of every step
uint8_t db_checkBP(db* ptr);

#endif //LC3SIMCORE_DEBUGGER_H
