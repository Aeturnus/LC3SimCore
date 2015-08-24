//
// Created by Brandon on 8/19/2015.
//

#include <stdint.h>
#include <stdlib.h>

#include "debugger.h"
#include "fileio.h"

void ldb_init(ldb *ptr, lc3 *lc3ptr)
{
    ptr->lc3ptr = lc3ptr;
    ptr->cycleCount = 0;
    ptr->breaker.mutex = 0;
    ptr->breaker.signal = 0;
    for(size_t i = 0; i < 0x10000; i++)
    {
        ldb_initBP(&(ptr->breakpoints[i]));
    }
}

enum CycleReturn ldb_step(ldb *ptr)
{
    if(lc3_cycle(ptr->lc3ptr) == HALT)
    {
        ldb_setBreak(ptr);
        return HALT;
    }
    ptr->cycleCount++;
    if(ldb_checkBP(ptr))
    {
        ldb_setBreak(ptr);
        return HALT;
    }
    return SUCCESS;
}
void ldb_stepOver(ldb *ptr)
{

}
void ldb_stepOut(ldb *ptr)
{

}
void ldb_run(ldb *ptr)
{
    ldb_clearBreak(ptr);
    while(!ldb_checkBreak(ptr))
    {
        ldb_step(ptr);
    }
    ldb_clearBreak(ptr);
}

inline uint8_t ldb_checkBreak(ldb *ptr)
{
    uint8_t* mutex = &ptr->breaker.mutex;
    while(*mutex);
    *mutex = 1;
    if(ptr->breaker.signal)
    {
        ptr->breaker.mutex = 0;
        return 1;
    }
    ptr->breaker.mutex = 0;
    return 0;
}
inline void ldb_setBreak(ldb *ptr)
{
    uint8_t* mutex = &ptr->breaker.mutex;
    while(*mutex);
    *mutex = 1;
    ptr->breaker.signal = 1;
    *mutex = 0;
}
inline void ldb_clearBreak(ldb *ptr)
{
    uint8_t* mutex = &ptr->breaker.mutex;
    while(*mutex);
    *mutex = 1;
    ptr->breaker.signal = 0;
    *mutex = 0;
}

void ldb_initBP(lbp *ptr)
{
    ptr->isCon = 0;
    ptr->isSet = 0;
    ptr->conType = bp_eq;
    ptr->srcType = bp_reg;
    ptr->srcNum = 0;
    ptr->testVal = 0;
}
uint8_t ldb_checkBP(ldb *ptr)
{
    lc3* lc3ptr = ptr->lc3ptr;
    lbp * point = &(ptr->breakpoints[lc3ptr->pc]);

    if(point->isSet)
    {
        if(point->isCon)
        {
            int16_t sourceVal;
            //Conditional
            if(point->srcType == bp_reg)
            {
                if(point->srcNum > 7)
                    return 0;   //Case catching for reg nums higher than 7
                sourceVal = (int16_t) (lc3ptr->registers[point->srcNum]);
            }
            else
            {
                sourceVal = (int16_t) (lc3ptr->mem[point->srcNum]);
            }
            switch(point->conType)
            {
                case bp_eq:
                    if(sourceVal == point->testVal)
                        return 1;
                    break;
                case bp_ne:
                    if(sourceVal != point->testVal)
                        return 1;
                    break;
                case bp_g:
                    if(sourceVal > point->testVal)
                        return 1;
                    break;
                case bp_l:
                    if(sourceVal < point->testVal)
                        return 1;
                    break;
                case bp_geq:
                    if(sourceVal >= point->testVal)
                        return 1;
                    break;
                case bp_leq:
                    if(sourceVal <= point->testVal)
                        return 1;
                    break;
            }
            return 0;
        }
        else
        {
            //Unconditional
            return 1;
        }
    }
    return 0;
}


uint8_t ldb_setBP(ldb *ptr, uint16_t address, uint8_t isConditional, uint8_t conditionalType, uint8_t sourceType,
                  uint16_t sourceNumber, int16_t testValue)
{
    //handle bad reg num
    if(sourceType == bp_reg)
    {
        if(sourceNumber > 7)
            return 0;
    }
    lbp * bpptr = &(ptr->breakpoints[address]);
    bpptr->isSet = 1;
    bpptr->isCon = isConditional;
    bpptr->conType = conditionalType;
    bpptr->srcType = sourceType;
    bpptr->srcNum = sourceNumber;
    bpptr->testVal = testValue;
    return 1;
}

uint8_t ldb_setUnconBP(ldb *ptr, uint16_t address)
{
    return ldb_setBP(ptr, address, 0, 0, 0, 0, 0);
}

//Loaders

//The classic loading scheme we all know and love
enum FileStatus ldb_loadObj(ldb* ptr, char* filePath)
{
    obj_file obj;
    enum FileStatus state = fio_loadObj(filePath,&obj);
    if(state != OPEN)
        return state;
    size_t i = 0;
    uint16_t entry = obj.buffer[i]; //Set the pc
    ptr->lc3ptr->pc = entry;
    uint16_t* mem = &(ptr->lc3ptr->mem[0]);  //Have a quick to access pointer to memory
    i++;
    for(; i < obj.size; i++)
    {
        if(entry+i > 0x10000)
            return NOTVALID;
        mem[entry+i-1] = obj.buffer[i];
    }

    fio_deleteObj(&obj);

    return SUCCESS;
}
