#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vdbesort_empty.h"



//
// Created by Waleed Rahman on 3/7/17.
//

#include "fpgaSorter.h"


//Mem createEmptyMemoryObject();
//void addRowToMemoryObject(Mem *mem, char * rowdata, int rowsize);
//void debugPrintfMemStr(Mem mem);

int main() {

    for (int i = 0; i < 15; i++) {
        fpgaSorterInsert(rand(), (int16_t )(rand() % 66535));
    }
    fpgaSorterSortLinear();
    printf("\nHello, World!\n");

    return 0;
}

//Mem createEmptyMemoryObject() {
//    struct Mem new_mem;
//    new_mem.z = 0;
//    new_mem.n = 0;
//    new_mem.zMalloc = 0;
//    new_mem.szMalloc = 0;
//    return new_mem;
//}
//
//void addRowToMemoryObject(Mem *mem, char * rowdata, int rowsize) {
////    printf("%d%d\n", rowdata[0], rowdata[1]);
//    if (mem->n == 0) {
//        mem->n = 2 + rowsize;
//        mem->szMalloc = 1200;
//        mem->zMalloc = (char*) malloc(1200 * sizeof(char));
//        mem->zMalloc[0] = (char)2;
//        mem->zMalloc[1] = (char)rowsize;
////        printf("\n--%d--", mem->zMalloc[1]);
//        memcpy (mem->zMalloc + 2, rowdata, sizeof(char));
//    }
//}
//
//
//void debugPrintfMemStr(Mem mem) {
//    printf("\n--Mem Structure--");
//    printf("\nMem Value: ");
//    printf("\n\tr: %f", mem.u.r);
//    printf("\n\tnZero: %d", mem.u.nZero);
//    printf("\n\tpDef: %p", (void *)mem.u.pDef);
//    printf("\n\tpRowSet: %p", (void *)mem.u.pRowSet);
//    printf("\n\tpFrame: %p", (void *)mem.u.pFrame);
//    printf("\nflags: %u", mem.flags);
//    printf("\nenc: %u", mem.enc);
//    printf("\neSubtype: %u", mem.eSubtype);
//    printf("\nn: %d", mem.n);
//    printf("\nz: %s", mem.z);
//    printf("\nzMalloc: %p", mem.zMalloc);
//    printf("\nszMalloc: %d", mem.szMalloc);
//    printf("\nMalloc Data: ");
//    for (int i = 0; i < mem.szMalloc; i++) {
//        printf("\ndata[%d]: %d", i, (int)*(mem.zMalloc + i));
//    }
//    printf("\nuTemp: %u", mem.uTemp);
//    printf("\ndb: %p", (void *)mem.db);
//    printf("\nxDel: %p", (void *)mem.xDel);
//
//}
