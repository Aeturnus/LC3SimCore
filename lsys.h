// "Little System" to represent a hypothetical LC3 motherboard
// Created by Brandon on 8/25/2015.
//

#ifndef LC3SIMCORE_LSYS_H
#define LC3SIMCORE_LSYS_H

#include "lc3.h"
#include "terminal.h"

typedef struct lsys_str
{
    lc3* lc3_ptr;
    term* term_ptr;
}lsys;

#endif //LC3SIMCORE_LSYS_H
