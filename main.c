#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vdbesort_empty.h"


/*
 *   Init()
 *   for each record: Write()
 *   Rewind()
 *     Rowkey()/Compare()
 *   Next()
 *   Close()
 *
 */

Mem createEmptyMemoryObject();
void addRowToMemoryObject(Mem *mem, char * rowdata, int rowsize);
void debugPrintfMemStr(Mem mem);

int main() {

    sqlite3VdbeSorterInit(0, 0, 0);
    char buffer [10];
    for (int i = 0; i < 7; i++) {
        Mem mem = createEmptyMemoryObject();

        for (int j = 0; j < 10; ++j) {
            printf("%d", buffer[j]);
        }
        printf("\n");
        int n = sprintf (buffer, "%d", (i+1) * 10);
        for (int j = 0; j < 10; ++j) {
            printf("%d", buffer[j]);
        }
        printf("\n");
        addRowToMemoryObject(&mem, buffer, 1);
//        debugPrintfMemStr(mem);
        sqlite3VdbeSorterWrite(0, &mem);
    }
    sqlite3VdbeSorterRewind(0, 0);
    for (int j = 0; j < 7; ++j) {
        sqlite3VdbeSorterRowkey(0, 0);
        sqlite3VdbeSorterNext(0, 0, 0);
    }
    sqlite3VdbeSorterClose(0, 0);
    sqlite3VdbeSorterReset(0, 0);
    printf("Hello, World!\n");

    return 0;
}

Mem createEmptyMemoryObject() {
    struct Mem new_mem;
    new_mem.z = 0;
    new_mem.n = 0;
    new_mem.zMalloc = 0;
    new_mem.szMalloc = 0;
    return new_mem;
}

void addRowToMemoryObject(Mem *mem, char * rowdata, int rowsize) {
    printf("%d%d\n", rowdata[0], rowdata[1]);
    if (mem->n == 0) {
        mem->n = 2 + rowsize;
        mem->szMalloc = 1200;
        mem->zMalloc = (char*) malloc(1200 * sizeof(char));
        mem->zMalloc[0] = (char)2;
        mem->zMalloc[1] = (char)rowsize;
//        printf("\n--%d--", mem->zMalloc[1]);
        memcpy (mem->zMalloc + 2, rowdata, sizeof(char));
    }
}


void debugPrintfMemStr(Mem mem) {
    printf("\n--Mem Structure--");
    printf("\nMem Value: ");
    printf("\n\tr: %f", mem.u.r);
    printf("\n\tnZero: %d", mem.u.nZero);
    printf("\n\tpDef: %p", (void *)mem.u.pDef);
    printf("\n\tpRowSet: %p", (void *)mem.u.pRowSet);
    printf("\n\tpFrame: %p", (void *)mem.u.pFrame);
    printf("\nflags: %u", mem.flags);
    printf("\nenc: %u", mem.enc);
    printf("\neSubtype: %u", mem.eSubtype);
    printf("\nn: %d", mem.n);
    printf("\nz: %s", mem.z);
    printf("\nzMalloc: %p", mem.zMalloc);
    printf("\nszMalloc: %d", mem.szMalloc);
    printf("\nMalloc Data: ");
    for (int i = 0; i < mem.szMalloc; i++) {
        printf("\ndata[%d]: %d", i, (int)*(mem.zMalloc + i));
    }
    printf("\nuTemp: %u", mem.uTemp);
    printf("\ndb: %p", (void *)mem.db);
    printf("\nxDel: %p", (void *)mem.xDel);

}