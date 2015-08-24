//
// Created by Brandon on 8/19/2015.
//

#ifndef LC3SIMCORE_DEBUGGER_H
#define LC3SIMCORE_DEBUGGER_H

#include <stdint.h>

#include "lc3.h"
#include "fileio.h"



//Conditional types
#define bp_eq   0
#define bp_ne   1
#define bp_neq  1
#define bp_g    2
#define bp_l    3
#define bp_geq  4
#define bp_leq  5
//Source type
#define bp_reg  0
#define bp_mem  1
//Breakpoint struct
typedef struct lbp_str
{
    uint8_t isSet;      //0: not set/1: set
    uint8_t isCon;      //0: unconditional/ 1: conditional
    uint8_t conType;    //Type of conditional
    uint8_t srcType;    //Type of source to check: mem or reg
    uint16_t srcNum;    //How it's addressed: reg 0-7 or mem 0x0000 - 0xFFFF
    int16_t testVal;    //Test value (signed)
} lbp;

typedef struct ldb_break_str
{
    uint8_t signal;     //Allows you to break out of a run/step over/step out
    uint8_t mutex;      //If 1, a lock has been set
} ldb_break;

//Debugger struct
typedef struct ldb_str
{
    struct lc3_str *lc3ptr;
    struct lbp_str breakpoints[0x10000];  //1:1 mapping of mem locs to breakpoints
    uint64_t cycleCount;
    ldb_break breaker;
} ldb;


void ldb_init(ldb *ptr, lc3 *lc3ptr);
//"step" will be a single cycle run. Others will call this. Returns 1 if breakpoint encountered
enum CycleReturn ldb_step(ldb *ptr);
//"stepOver" will be a single cycle run unless it's a JSR/JSRR call that will continue running until PC equals the next instruction
void ldb_stepOver(ldb *ptr);
//"stepOut" will run until JMP R7 is hit
void ldb_stepOut(ldb *ptr);
//"run" will continue until a breakpoint is hit
void ldb_run(ldb *ptr);

//Breaker stuff

//returns 1 if a break has occurred; 0 if it's still running
uint8_t ldb_checkBreak(ldb *ptr);
//Setting break will cause the machine to stop
void ldb_setBreak(ldb *ptr);
//Clearing break will ensure running the machine
void ldb_clearBreak(ldb *ptr);

//This will be run at the end of every step. Returns 1 if breakpoint encountered
void ldb_initBP(lbp *ptr);


//Breakpoint stuff
//Checks if there's a breakpoint
uint8_t ldb_checkBP(ldb *ptr);
//Master breakpoint setter
uint8_t ldb_setBP(ldb *ptr, uint16_t address, uint8_t isConditional, uint8_t conditionalType, uint8_t sourceType,
                  uint16_t sourceNumber, int16_t testValue);
//Convenient setter for unconditional breakpoints
uint8_t ldb_setUnconBP(ldb *ptr, uint16_t address);
//Convenient setter for conditional breakpoints
uint8_t ldb_setConBP(ldb *ptr, uint16_t address, uint8_t conditionalType, uint8_t sourceType, uint16_t sourceNumber,
                     int16_t testValue);


//Loaders
enum FileStatus ldb_loadObj(ldb* ptr, char* filePath);
enum FileStatus ldb_loadO(ldb* ptr, char* filePath);

//Command interpreter

#endif //LC3SIMCORE_DEBUGGER_H
