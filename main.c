#include <stdio.h>
#include <stdlib.h>
#include "lc3.h"

enum FileStatus
{
    OPEN,
    NOTVALID,
    PARITY
};

enum FileStatus openObj(char* filePath,obj_file* obj);
void deleteObj(obj_file* obj);

int main() {
    unsigned int cycles = 0;

    printf("Hello world\n");
    LC3 lc3;
    LC3_init(&lc3);

    obj_file obj ={0,0,0};

    enum FileStatus state = openObj("testcases/lsi.obj",&obj);
    if(state != OPEN)
    {
        printf("FAIL!\n");
        return -1;
    }
    LC3_loadObj(&lc3,obj);

    for(lc3.memory.mem[0xFFFE] |= 0x8000 ; LC3_cycle(&lc3) == SUCCESS;cycles++);
    for(int i = 0; i < GPR_NUM; i++)
    {
        printf("Reg %d: x%04x | %d\n",i,lc3.registers[i],(int16_t)lc3.registers[i]);
    }
    for(int i = 0; i < 8; i++)
    {
        printf("x%x : x%04x\n",0x3000+i,lc3.memory.mem[0x3000+i]);
    }
    printf("%d cycles\n",cycles);


    return 0;
}


enum FileStatus openObj(char* filePath,obj_file* ptr)
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
        fread(&((uint8_t*)ptr->buffer)[i+1],1,1,f);
        fread(&((uint8_t*)ptr->buffer)[i],1,1,f);
        ptr->size++;
        //fread(&ptr->buffer[i],2,1,f);
    }
    fclose(f);
    ptr->allocated = 1;
    return SUCCESS;
}

void deleteObj(obj_file* obj)
{
    if(obj->allocated)
    {
        free(obj->buffer);
    }
}
