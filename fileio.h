//
// Created by Brandon on 8/19/2015.
//


#ifndef LC3SIMCORE_FILEIO_H
#define LC3SIMCORE_FILEIO_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

typedef enum FileStatus_e
{
    OPEN,
    NOTVALID,
    PARITY
} FileStatus;

typedef struct obj_str
{
    size_t size;
    uint16_t* buffer;
    uint8_t allocated;
}obj_file;

//"sub o" struct
typedef struct subo_str
{
    uint16_t entryPoint;
    uint16_t size;
    uint16_t* text;         //Text segment
}o_sub;

typedef struct o_str
{
    uint16_t primaryEntryPoint;
    uint16_t objNum;
    o_sub* subos;
}o_file;


//Virtual disk file
typedef struct vdsk_str
{
    uint32_t size;      //Indicates size in bytes of the disk
    uint8_t* data;      //Gotta load it all into memory
    uint8_t allocated;
}vdsk_file;

enum FileStatus_e fio_loadObj(char *filePath, obj_file *ptr);
void fio_deleteObj(obj_file* ptr);

enum FileStatus_e fio_loadO(char *filePath, o_file *ptr);
void fio_deleteO(o_file* ptr);

void fio_loadSubO(FILE* file, o_sub *ptr);
void fio_deleteSubO(o_sub *ptr);

enum FileStatus_e fio_loadVdsk(char *filePath, vdsk_file *ptr);
void fio_deleteVdsk(vdsk_file *ptr);
enum FileStatus_e fio_makeNewVdsk(char *filePath, uint32_t size);


//be: big endian/ le: little endian
uint16_t fio_read16_be(FILE* file);
uint16_t fio_read16_le(FILE* file);
void fio_write16_be(FILE* file,uint16_t input);
void fio_write16_le(FILE* file,uint16_t input);

uint32_t fio_read32_be(FILE* file);
uint32_t fio_read32_le(FILE* file);
void fio_write32_be(FILE* file,uint32_t input);
void fio_write32_le(FILE* file,uint32_t input);

/*
void fio_read16_be_p(FILE* file, uint16_t* );
void fio_read16_le_p(FILE* file);
void fio_write16_be_p(FILE* file,uint16_t input);
void fio_write16_le_p(FILE* file,uint16_t input);
 */
#endif //LC3SIMCORE_FILEIO_H
