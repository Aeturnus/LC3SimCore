//
// Created by Brandon on 8/19/2015.
//

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "fileio.h"
#include "lc3.h"

enum FileStatus fio_loadObj(char *filePath, obj_file *ptr)
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

enum FileStatus fio_loadO(char *filePath, o_file *ptr)
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


uint16_t fio_read16_be(FILE* file)
{
    uint16_t num;
    uint8_t* num_ptr = (uint8_t*)&num;
    fread(&num_ptr[1],1,1,file);
    fread(&num_ptr[0],1,1,file);
    return num;
}
uint16_t fio_read16_le(FILE* file)
{
    uint16_t num;
    fread(&num,2,1,file);
    return num;
}
void fio_write16_be(FILE* file,uint16_t input)
{
    //fwrite(&((uint8_t*)(&input)[1]),1,1,file);
    //fwrite(&((uint8_t*)(&input)[0]),1,1,file);
}
void fio_write16_le(FILE* file,uint16_t input)
{
    fwrite(&input,2,1,file);
}

