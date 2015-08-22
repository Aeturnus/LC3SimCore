//
// Created by Brandon on 8/19/2015.
//

#include <stdint.h>
#include <stdlib.h>

#include "debugger.h"
#include "fileio.h"
void db_loadObj(char* filePath, db* ptr)
{
    obj_file obj;
    if(fio_loadObj(filePath, &obj) != OPEN)
        return;
    lc3_loadObj(ptr->lc3ptr,obj);
    fio_deleteObj(&obj);
}


uint8_t db_step(db* ptr)
{
    lc3_cycle(ptr->lc3ptr);
    ptr->cycleCount++;
    return db_checkBP(ptr);
}
void db_stepOver(db* ptr)
{

}
void db_stepOut(db* ptr)
{

}
void db_run(db* ptr)
{
    while(db_step(ptr));
}

uint8_t db_checkBP(db* ptr)
{
    lc3* lc3ptr = ptr->lc3ptr;
    bp* point = &(ptr->breakpoints[lc3ptr->mem[lc3ptr->pc]]);

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
