//
// Created by Brandon on 8/19/2015.
//

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "fileio.h"
#include "lc3.h"

enum FileStatus_e fio_loadObj(char *filePath, obj_file *ptr)
{
    FILE* f;
    f = fopen(filePath,"rb");
    if(f == NULL)
    {
        fclose(f);
        return NOTVALID;
    }
    fseek(f,0,SEEK_END);
    size_t size = ftell(f);
    if(size%2 == 1)
    {
        fclose(f);
        return PARITY;
    }
    fseek(f,0,SEEK_SET);

    if(ptr->allocated)
    {
        free(ptr->buffer);
    }
    ptr->size = 0;
    ptr->buffer = malloc(size);    //size is in bytes
    for(size_t i = 0; i < size;i+=2)
    {
        //Since it's big endian, we have to hack around it
        ptr->buffer[i/2] = fio_read16_be(f);
        ptr->size++;
        //fread(&ptr->buffer[i],2,1,f);
    }
    fclose(f);
    ptr->allocated = 1;
    return SUCCESS;
}

void fio_deleteObj(obj_file* obj)
{
    if(obj->allocated)
    {
        free(obj->buffer);
    }
}

enum FileStatus_e fio_loadO(char *filePath, o_file *ptr)
{
    FILE* f;
    f = fopen(filePath,"rb");
    if(f == NULL)
    {
        fclose(f);
        return NOTVALID;
    }
    ptr->primaryEntryPoint = fio_read16_be(f);
    ptr->objNum = fio_read16_be(f);
    ptr->subos = malloc(sizeof(struct subo_str) * ptr->objNum); //Allocate space for each subo
    for(size_t i = 0; i < ptr->objNum; i++)
    {
        fio_loadSubO(f,&ptr->subos[i]);     //Load the subos
    }
    return SUCCESS;
}
void fio_deleteO(o_file* ptr)
{
    for(size_t i = 0; i < ptr->objNum; i++)
    {
        fio_deleteSubO(&ptr->subos[i]);
    }
    free(ptr->subos);
}

void fio_loadSubO(FILE* file, o_sub *ptr)
{
    ptr->entryPoint = fio_read16_be(file);
    ptr->size = fio_read16_be(file);
    ptr->text = malloc(sizeof(uint16_t) * ptr->size);
    for(size_t i = 0; i < ptr->size; i++)
    {
        ptr->text[i] = fio_read16_be(file);
    }
}

void fio_deleteSubO(o_sub *ptr)
{
    free(ptr->text);
}

enum FileStatus_e fio_loadVdsk(char *filePath, vdsk_file *ptr)
{
    if(ptr->allocated)
        fio_deleteVdsk(ptr);
    FILE* f = fopen(filePath,"rb");
    if(f == NULL)
        return NOTVALID;
    ptr->size = fio_read16_le(f);
    ptr->data = malloc(ptr->size);      //Allocate memory for the ptr
    ptr->allocated = 1;
    fread(ptr->data,1,ptr->size,f);     //Read ptr->size bytes from the file
    return OPEN;
}
void fio_deleteVdsk(vdsk_file *ptr)
{
    if(ptr->allocated)
    {
        free(ptr->data);
        ptr->size = 0;
        ptr->allocated = 1;
    }
}
enum FileStatus_e fio_makeNewVdsk(char *filePath, uint32_t size)
{
    FILE* f = fopen(filePath,"wb");
    if(f == NULL)
        return NOTVALID;
    fio_write32_le(f,size);
    uint8_t init = 0;
    for(int i = 0; i < size; i++)
    {
        fwrite(&init, 1, 1, f);
    }
    return OPEN;
}

//Read/write
inline uint16_t fio_read16_be(FILE* file)
{
    uint16_t num;
    uint8_t* num_ptr = (uint8_t*)&num;
    fread(&num_ptr[1],1,1,file);
    fread(&num_ptr[0],1,1,file);
    return num;
}
inline uint16_t fio_read16_le(FILE* file)
{
    uint16_t num;
    fread(&num,2,1,file);
    return num;
}
inline void fio_write16_be(FILE* file,uint16_t input)
{
    uint8_t* num_ptr = (uint8_t*)&input;
    fwrite(&num_ptr[1],1,1,file);
    fwrite(&num_ptr[0],1,1,file);
}
inline void fio_write16_le(FILE* file,uint16_t input)
{
    fwrite(&input,2,1,file);
}

inline uint32_t fio_read32_be(FILE* file)
{
    uint32_t num;
    uint8_t* num_ptr = (uint8_t*)&num;
    fread(&num_ptr[3],1,1,file);
    fread(&num_ptr[2],1,1,file);
    fread(&num_ptr[1],1,1,file);
    fread(&num_ptr[0],1,1,file);
    return num;
}
inline uint32_t fio_read32_le(FILE* file)
{
    uint32_t num;
    fread(&num,4,1,file);
    return num;
}
inline void fio_write32_be(FILE* file,uint32_t input)
{
    uint8_t* num_ptr = (uint8_t*)&input;
    fwrite(&num_ptr[3],1,1,file);
    fwrite(&num_ptr[2],1,1,file);
    fwrite(&num_ptr[1],1,1,file);
    fwrite(&num_ptr[0],1,1,file);
}
inline void fio_write32_le(FILE* file,uint32_t input)
{
    fwrite(&input,4,1,file);
}
