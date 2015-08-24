#include <stdio.h>
#include <stdlib.h>

#include "fileio.h"
#include "lc3.h"
#include "ldb.h"

int debuggerTest();
int otherTest();
int test();

void keyListener(void)
{
    int in = getchar();
    if(in == 27)
        return;
}


int main()
{
    //printf("Select: \n(a) Step\n(b) Provide keyboard input\n(c) Stop");
    //test();
    otherTest();
    return 0;
}

int otherTest()
{
    lc3 lc3;
    lc3_init(&lc3);
    ldb db;
    ldb_init(&db, &lc3);
    if(ldb_loadObj(&db,"testcases/Project1.obj")!=OPEN)
        return -1;
    lc3.mem[0x30f0] = 25;
    lc3.mem[0x30f1] = 4;
    //ldb_setUnconBP(&db,0x3001);
    //ldb_setBP(&db, 0x3001, 1, bp_neq, bp_reg, 0, 0xbeef);
    ldb_run(&db);

    for(int i = 0; i < GPR_NUM; i++)
    {
        printf("R%d    : x%04x | %d\n",i,lc3.registers[i],(int16_t)lc3.registers[i]);
    }
    for(int i = 0; i < 8; i++)
    {
        printf("x%x : x%04x | %d\n",0x3000+i,lc3.mem[0x3000+i],(int16_t)lc3.mem[0x3000+i]);
    }
    printf("x%x : x%04x | %d\n",0x30f2,lc3.mem[0x30f2],(int16_t)lc3.mem[0x30f2]);
    printf("%llu cycles\n",(long long unsigned int)db.cycleCount);

    return 0;
}

int debuggerTest()
{
    lc3 lc3;
    lc3_init(&lc3);
    ldb db;
    ldb_init(&db, &lc3);
    if(ldb_loadObj(&db,"testcases/lsi.obj") != OPEN)
        return -1;
    //ldb_setUnconBP(&db,0x3001);
    ldb_setBP(&db, 0x3001, 1, bp_neq, bp_reg, 0, 0xbeef);
    ldb_run(&db);

    for(int i = 0; i < GPR_NUM; i++)
    {
        printf("R%d    : x%04x | %d\n",i,lc3.registers[i],(int16_t)lc3.registers[i]);
    }
    for(int i = 0; i < 8; i++)
    {
        printf("x%x : x%04x | %d\n",0x3000+i,lc3.mem[0x3000+i],(int16_t)lc3.mem[0x3000+i]);
    }
    printf("%llu cycles\n",(long long unsigned int) db.cycleCount);

    return 0;
}

int test() {
    unsigned int cycles = 0;

    printf("Hello world\n");
    lc3 lc3;
    lc3_init(&lc3);

    for(lc3.mem[0xFFFE] |= 0x8000 ; lc3_cycle(&lc3) != HALT;cycles++);
    for(int i = 0; i < GPR_NUM; i++)
    {
        printf("R%d    : x%04x | %d\n",i,lc3.registers[i],(int16_t)lc3.registers[i]);
    }
    for(int i = 0; i < 8; i++)
    {
        printf("x%x : x%04x | %d\n",0x3000+i,lc3.mem[0x3000+i],(int16_t)lc3.mem[0x3000+i]);
    }
    printf("%d cycles\n",cycles);


    return 0;
}
