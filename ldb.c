//
// Created by Brandon on 8/19/2015.
//

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "ldb.h"
#include "fileio.h"


void ldb_init(ldb *ptr, lc3 *lc3ptr) {
    ptr->lc3ptr = lc3ptr;
    ptr->cycleCount = 0;
    ptr->breaker.mutex = 0;
    ptr->breaker.signal = 0;
    for (size_t i = 0; i < 0x10000; i++) {
        ldb_initBP(&(ptr->breakpoints[i]));
    }
}

void ldb_newlc3(ldb *ptr) {
    lc3 *newlc3 = malloc(sizeof(struct lc3_str));
    lc3_init(newlc3);
    ldb_init(ptr, newlc3);
}

ldb *ldb_new(void) {
    ldb *newldb = malloc(sizeof(struct ldb_str));
    ldb_newlc3(newldb);
    return newldb;
}

enum CycleReturn_e ldb_step(ldb *ptr) {
    if (lc3_cycle(ptr->lc3ptr) == HALT) {
        ldb_setBreak(ptr);
        return HALT;
    }
    ptr->cycleCount++;
    if (ldb_checkBP(ptr)) {
        ldb_setBreak(ptr);
        return HALT;
    }
    return SUCCESS;
}

void ldb_stepOver(ldb *ptr) {

}

void ldb_stepOut(ldb *ptr) {

}

void ldb_run(ldb *ptr) {
    ldb_clearBreak(ptr);
    while (!ldb_checkBreak(ptr)) {
        ldb_step(ptr);
    }
    ldb_clearBreak(ptr);
}

inline uint8_t ldb_checkBreak(ldb *ptr) {
    uint8_t *mutex = &ptr->breaker.mutex;
    while (*mutex);
    *mutex = 1;
    if (ptr->breaker.signal) {
        ptr->breaker.mutex = 0;
        return 1;
    }
    ptr->breaker.mutex = 0;
    return 0;
}

inline void ldb_setBreak(ldb *ptr) {
    uint8_t *mutex = &ptr->breaker.mutex;
    while (*mutex);
    *mutex = 1;
    ptr->breaker.signal = 1;
    *mutex = 0;
}

inline void ldb_clearBreak(ldb *ptr) {
    uint8_t *mutex = &ptr->breaker.mutex;
    while (*mutex);
    *mutex = 1;
    ptr->breaker.signal = 0;
    *mutex = 0;
}

void ldb_initBP(lbp *ptr) {
    ptr->isCon = 0;
    ptr->isSet = 0;
    ptr->conType = ldb_eq;
    ptr->srcType = ldb_reg;
    ptr->srcNum = 0;
    ptr->testVal = 0;
}

uint8_t ldb_checkBP(ldb *ptr) {
    lc3 *lc3ptr = ptr->lc3ptr;
    lbp *point = &(ptr->breakpoints[lc3ptr->pc]);

    if (point->isSet) {
        if (point->isCon) {
            int16_t sourceVal;
            //Conditional
            if (point->srcType == ldb_reg) {
                if (point->srcNum > 7)
                    return 0;   //Case catching for reg nums higher than 7
                sourceVal = (int16_t) (lc3ptr->registers[point->srcNum]);
            }
            else {
                sourceVal = (int16_t) (lc3ptr->mem[point->srcNum]);
            }
            switch (point->conType) {
                case ldb_eq:
                    if (sourceVal == point->testVal)
                        return 1;
                    break;
                case ldb_ne:
                    if (sourceVal != point->testVal)
                        return 1;
                    break;
                case ldb_g:
                    if (sourceVal > point->testVal)
                        return 1;
                    break;
                case ldb_l:
                    if (sourceVal < point->testVal)
                        return 1;
                    break;
                case ldb_geq:
                    if (sourceVal >= point->testVal)
                        return 1;
                    break;
                case ldb_leq:
                    if (sourceVal <= point->testVal)
                        return 1;
                    break;
            }
            return 0;
        }
        else {
            //Unconditional
            return 1;
        }
    }
    return 0;
}


uint8_t ldb_setBP(ldb *ptr, uint16_t address, uint8_t isConditional, uint8_t conditionalType, uint8_t sourceType,
                  uint16_t sourceNumber, int16_t testValue) {
    //handle bad reg num
    if (sourceType == ldb_reg) {
        if (sourceNumber > 7)
            return 0;
    }
    lbp *bpptr = &(ptr->breakpoints[address]);
    bpptr->isSet = 1;
    bpptr->isCon = isConditional;
    bpptr->conType = conditionalType;
    bpptr->srcType = sourceType;
    bpptr->srcNum = sourceNumber;
    bpptr->testVal = testValue;
    return 1;
}

uint8_t ldb_setUnconBP(ldb *ptr, uint16_t address) {
    return ldb_setBP(ptr, address, 0, 0, 0, 0, 0);
}


//Setters and getters
uint8_t ldb_set(ldb *ptr, uint8_t srcType, uint16_t srcNum, uint16_t setVal) {
    if (srcType == ldb_reg && srcNum > ldb_reg_max)
        return 0;   //Fail if invalid address for srcNum
    if (srcType == ldb_mem)
        ldb_setMem(ptr, srcNum, setVal);
    else if (srcType == ldb_reg)
        ldb_setReg(ptr, srcNum, setVal);
    return 1;
}

inline void ldb_setReg(ldb *ptr, uint16_t srcNum, uint16_t setVal) {
    switch (srcNum) {
        case ldb_reg_psr:
            ptr->lc3ptr->psr = setVal;
            break;
        case ldb_reg_pc:
            ptr->lc3ptr->pc = setVal;
            break;
        case ldb_reg_ir:
            ptr->lc3ptr->ir = setVal;
            break;
        case ldb_reg_mar:
            ptr->lc3ptr->mar = setVal;
            break;
        case ldb_reg_mdr:
            ptr->lc3ptr->mdr = setVal;
            break;
        default:
            ptr->lc3ptr->registers[srcNum] = setVal;
    }
}

inline void ldb_setMem(ldb *ptr, uint16_t srcNum, uint16_t setVal) {
    ptr->lc3ptr->mem[srcNum] = setVal;
}

uint16_t ldb_get(ldb *ptr, uint8_t srcType, uint16_t srcNum) {
    if (srcType == ldb_reg && srcNum > ldb_reg_max)
        return 0;   //Fail if invalid address for srcNum
    if (srcType == ldb_mem)
        ldb_getMem(ptr, srcNum);
    else if (srcType == ldb_reg)
        ldb_getReg(ptr, srcNum);
    return 1;
}

inline uint16_t ldb_getReg(ldb *ptr, uint16_t srcNum) {
    switch (srcNum) {
        case ldb_reg_psr:
            return ptr->lc3ptr->psr;
            break;
        case ldb_reg_pc:
            return ptr->lc3ptr->pc;
            break;
        case ldb_reg_ir:
            return ptr->lc3ptr->ir;
            break;
        case ldb_reg_mar:
            return ptr->lc3ptr->mar;
            break;
        case ldb_reg_mdr:
            return ptr->lc3ptr->mdr;
            break;
        default:
            return ptr->lc3ptr->registers[srcNum];
    }
}

inline uint16_t ldb_getMem(ldb *ptr, uint16_t srcNum) {
    return ptr->lc3ptr->mem[srcNum];
}

void ldb_setCC(ldb *ptr, int set) {
    uint8_t bits = 0;
    if (set < 0)
        bits = 0x04;
    else if (set == 0)
        bits = 0x02;
    else
        bits = 0x01;
    ptr->lc3ptr->psr &= ~0x7;   //Clear CC
    ptr->lc3ptr->psr |= bits;
}

//Output: -1 for N, 0 for Z, 1 for P
int ldb_getCC(ldb *ptr) {
    uint16_t bits = ptr->lc3ptr->psr &= 0x7;
    switch (bits) {
        case 0x01:
            return 1;
        case 0x02:
            return 0;
        case 0x03:
            return -1;
        default:
            return 0;
    }
}

//Set/get privelege
void ldb_setPrivilege(ldb *ptr, uint8_t usermode)
{
    if(usermode)
        ptr->lc3ptr->psr |= 0x8000;
    else
        ptr->lc3ptr->psr &= 0x7FFF;
}
uint8_t ldb_getPrivilege(ldb *ptr)
{
    return (uint8_t)(ptr->lc3ptr->psr >> 15) & 0xFF;
}



//Loaders

//The classic loading scheme we all know and love
enum FileStatus_e ldb_loadObj(ldb *ptr, char *filePath) {
    obj_file obj;
    enum FileStatus_e state = fio_loadObj(filePath, &obj);
    if (state != OPEN)
        return state;
    size_t i = 0;
    uint16_t entry = obj.buffer[i]; //Set the pc
    ptr->lc3ptr->pc = entry;
    uint16_t *mem = &(ptr->lc3ptr->mem[0]);  //Have a quick to access pointer to memory
    i++;
    for (; i < obj.size; i++) {
        if (entry + i > 0x10000)
            return NOTVALID;
        mem[entry + i - 1] = obj.buffer[i];
    }

    fio_deleteObj(&obj);

    return OPEN;
}

//DUMPS
enum FileStatus_e ldb_dump_lc3_r(ldb *ldbptr, char *filePath) {
    FILE *file = fopen(filePath, "w");    //Open a writable file
    if (file == NULL)
        return NOTVALID;
    lc3 *ptr = ldbptr->lc3ptr;
    //Dump spec registers
    fprintf(file,
            "PSR   : x%04x | %d\nPC    : x%04x | %d\nIR    : x%04x | %d\nMAR   : x%04x | %d\nMDR   : x%04x | %d\n",
            ptr->psr, (int16_t) ptr->psr, ptr->pc, (int16_t) ptr->pc, ptr->ir, (int16_t) ptr->ir, ptr->mar,
            (int16_t) ptr->mar, ptr->mdr, (int16_t) ptr->mdr);

    //Dump gen registers
    uint16_t *reg = ptr->registers;
    for (int i = 0; i < 8; i++) {
        fprintf(file, "R%d    : x%04x | %d\n", i, reg[i], (int16_t) reg[i]);
    }
    //Dump memory
    for (int i = 0; i < 0x10000; i++) {
        fprintf(file, "x%04x : x%04x | %d\n", i, ptr->mem[i], (int16_t) ptr->mem[i]);
    }

    //Dump interrupt lines
    for (int i = 0; i < 32; i++) {
        fprintf(file, "INT%02d:%04x\n", i, ptr->intLines[i]);
    }
    return OPEN;
}

enum FileStatus_e ldb_dump_lc3_t(ldb *ldbptr, char *filePath) {
    FILE *file = fopen(filePath, "w");    //Open a writable file
    if (file == NULL)
        return NOTVALID;
    lc3 *ptr = ldbptr->lc3ptr;

    //Structure is inverted for easy access to memory

    //Dump memory
    for (int i = 0; i < 0x10000; i++) {
        fprintf(file, "%04x\n", ptr->mem[i]);
    }

    //Dump gen registers
    uint16_t *reg = ptr->registers;
    fprintf(file, "%04x\n%04x\n%04x\n%04x\n%04x\n%04x\n%04x\n%04x\n", reg[0], reg[1], reg[2], reg[3], reg[4], reg[5],
            reg[6], reg[7]);

    //Dump interrupt lines
    for (int i = 0; i < 32; i++) {
        fprintf(file, "%04x\n", ptr->intLines[i]);
    }

    //Dump spec registers
    fprintf(file, "%04x\n%04x\n%04x\n%04x\n%04x\n", ptr->psr, ptr->pc, ptr->ir, ptr->mar, ptr->mdr);


    return OPEN;
}

enum FileStatus_e ldb_dump_lc3_b(ldb *ldbptr, char *filePath) {
    FILE *file = fopen(filePath, "wb");
    if (file == NULL)
        return NOTVALID;
    lc3 *ptr = ldbptr->lc3ptr;
    //Dump spec registers
    fio_write16_le(file, ptr->psr);
    fio_write16_le(file, ptr->pc);
    fio_write16_le(file, ptr->ir);
    fio_write16_le(file, ptr->mar);
    fio_write16_le(file, ptr->mdr);
    //Dump interrupt lines
    for (int i = 0; i < 32; i++) {
        fwrite(&ptr->intLines[i], 1, 1, file);
    }
    //Dump gen registers
    for (int i = 0; i < 8; i++) {
        fio_write16_le(file, ptr->registers[i]);
    }
    //Dump memory
    for (int i = 0; i < 0x10000; i++) {
        fio_write16_le(file, ptr->mem[i]);
    }
    return OPEN;
}

enum FileStatus_e ldb_dumpload_lc3_b(ldb *ldbptr, char *filePath) {
    FILE *file = fopen(filePath, "rb");
    if (file == NULL)
        return NOTVALID;
    lc3 *ptr = ldbptr->lc3ptr;
    //SPRs
    ptr->psr = fio_read16_le(file);
    ptr->pc = fio_read16_le(file);
    ptr->ir = fio_read16_le(file);
    ptr->mar = fio_read16_le(file);
    ptr->mdr = fio_read16_le(file);
    //Int lines
    for (int i = 0; i < 32; i++) {
        fread(&ptr->intLines[i], 1, 1, file);
    }
    //Gen regs
    for (int i = 0; i < 8; i++) {
        ptr->registers[i] = fio_read16_le(file);
    }
    //Mem
    for (int i = 0; i < 0x10000; i++) {
        ptr->mem[i] = fio_read16_le(file);
    }
    return OPEN;
}


#ifdef __cplusplus
}
#endif
