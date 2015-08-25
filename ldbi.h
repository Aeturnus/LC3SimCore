// ldb interpreter
// Created by Brandon on 8/24/2015.
//

#ifndef LC3SIMCORE_LDBI_H
#define LC3SIMCORE_LDBI_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>      //Creates output files for last command
#include <string.h>

#include "ldb.h"
#include "lc3.h"

uint8_t ldbi_interpret(ldb* ldbptr,char* commandString);

#endif //LC3SIMCORE_LDBI_H
