#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vdbesort_empty.h"



//
// Created by Waleed Rahman on 3/7/17.
//

#include "fpgaSorter.h"

/*
** 2011-07-09
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This file contains code for the VdbeSorter object, used in concert with
** a VdbeCursor to sort large numbers of keys for CREATE INDEX statements
** or by SELECT statements with ORDER BY clauses that cannot be satisfied
** using indexes and without LIMIT clauses.
**
** The VdbeSorter object implements a multi-threaded external merge sort
** algorithm that is efficient even if the number of elements being sorted
** exceeds the available memory.
**
** Here is the (internal, non-API) interface between this module and the
** rest of the SQLite system:
**
**    sqlite3VdbeSorterInit()       Create a new VdbeSorter object.
**
**    sqlite3VdbeSorterWrite()      Add a single new row to the VdbeSorter
**                                  object.  The row is a binary blob in the
**                                  OP_MakeRecord format that contains both
**                                  the ORDER BY key columns and result columns
**                                  in the case of a SELECT w/ ORDER BY, or
**                                  the complete record for an index entry
**                                  in the case of a CREATE INDEX.
**
**    sqlite3VdbeSorterRewind()     Sort all content previously added.
**                                  Position the read cursor on the
**                                  first sorted element.
**
**    sqlite3VdbeSorterNext()       Advance the read cursor to the next sorted
**                                  element.
**
**    sqlite3VdbeSorterRowkey()     Return the complete binary blob for the
**                                  row currently under the read cursor.
**
**    sqlite3VdbeSorterCompare()    Compare the binary blob for the row
**                                  currently under the read cursor against
**                                  another binary blob X and report if
**                                  X is strictly less than the read cursor.
**                                  Used to enforce uniqueness in a
**                                  CREATE UNIQUE INDEX statement.
**
**    sqlite3VdbeSorterClose()      Close the VdbeSorter object and reclaim
**                                  all resources.
**
**    sqlite3VdbeSorterReset()      Refurbish the VdbeSorter for reuse.  This
**                                  is like Close() followed by Init() only
**                                  much faster.
**
** The interfaces above must be called in a particular order.  Write() can
** only occur in between Init()/Reset() and Rewind().  Next(), Rowkey(), and
** Compare() can only occur in between Rewind() and Close()/Reset(). i.e.
**
**   Init()
**   for each record: Write()
**   Rewind()
**     Rowkey()/Compare()
**   Next()
**   Close()
*/
//#include "sqliteInt.h"
//#include "vdbeInt.h"

/*
** Hard-coded maximum amount of data to accumulate in memory before flushing
** to a level 0 PMA. The purpose of this limit is to prevent various integer
** overflows. 512MiB.
*/
#define SQLITE_MAX_PMASZ    (1<<29)

/*
** Private objects used by the sorter
*/
typedef struct MergeEngine MergeEngine;     /* Merge PMAs together */
typedef struct PmaReader PmaReader;         /* Incrementally read one PMA */
typedef struct PmaWriter PmaWriter;         /* Incrementally write one PMA */
typedef struct SorterRecord SorterRecord;   /* A record being sorted */
typedef struct SortSubtask SortSubtask;     /* A sub-task in the sort process */
typedef struct SorterFile SorterFile;       /* Temporary file object wrapper */
typedef struct SorterList SorterList;       /* In-memory list of records */
typedef struct IncrMerger IncrMerger;       /* Read & merge multiple PMAs */

struct SorterFile {
    sqlite3_file *pFd;              /* File handle */
    i64 iEof;                       /* Bytes of data stored in pFd */
};

struct SorterList {
    SorterRecord *pList;            /* Linked list of records */
    u8 *aMemory;                    /* If non-NULL, bulk memory to hold pList */
    int szPMA;                      /* Size of pList as PMA in bytes */
};

struct MergeEngine {
    int nTree;                 /* Used size of aTree/aReadr (power of 2) */
    SortSubtask *pTask;        /* Used by this thread only */
    int *aTree;                /* Current state of incremental merge */
    PmaReader *aReadr;         /* Array of PmaReaders to merge data from */
};

/*
** This object represents a single thread of control in a sort operation.
** Exactly VdbeSorter.nTask instances of this object are allocated
** as part of each VdbeSorter object. Instances are never allocated any
** other way. VdbeSorter.nTask is set to the number of worker threads allowed
** (see SQLITE_CONFIG_WORKER_THREADS) plus one (the main thread).  Thus for
** single-threaded operation, there is exactly one instance of this object
** and for multi-threaded operation there are two or more instances.
**
** Essentially, this structure contains all those fields of the VdbeSorter
** structure for which each thread requires a separate instance. For example,
** each thread requries its own UnpackedRecord object to unpack records in
** as part of comparison operations.
**
** Before a background thread is launched, variable bDone is set to 0. Then,
** right before it exits, the thread itself sets bDone to 1. This is used for
** two purposes:
**
**   1. When flushing the contents of memory to a level-0 PMA on disk, to
**      attempt to select a SortSubtask for which there is not already an
**      active background thread (since doing so causes the main thread
**      to block until it finishes).
**
**   2. If SQLITE_DEBUG_SORTER_THREADS is defined, to determine if a call
**      to sqlite3ThreadJoin() is likely to block. Cases that are likely to
**      block provoke debugging output.
**
** In both cases, the effects of the main thread seeing (bDone==0) even
** after the thread has finished are not dire. So we don't worry about
** memory barriers and such here.
*/
typedef int (*SorterCompare)(SortSubtask*,int*,const void*,int,const void*,int);
struct SortSubtask {
    SQLiteThread *pThread;          /* Background thread, if any */
    int bDone;                      /* Set if thread is finished but not joined */
    VdbeSorter *pSorter;            /* Sorter that owns this sub-task */
    UnpackedRecord *pUnpacked;      /* Space to unpack a record */
    SorterList list;                /* List for thread to write to a PMA */
    int nPMA;                       /* Number of PMAs currently in file */
    SorterCompare xCompare;         /* Compare function to use */
    SorterFile file;                /* Temp file for level-0 PMAs */
    SorterFile file2;               /* Space for other PMAs */
};


struct VdbeSorter {
    int mnPmaSize;                  /* Minimum PMA size, in bytes */
    int mxPmaSize;                  /* Maximum PMA size, in bytes.  0==no limit */
    int mxKeysize;                  /* Largest serialized key seen so far */
    int pgsz;                       /* Main database page size */
    PmaReader *pReader;             /* Readr data from here after Rewind() */
    MergeEngine *pMerger;           /* Or here, if bUseThreads==0 */
    sqlite3 *db;                    /* Database connection */
    KeyInfo *pKeyInfo;              /* How to compare records */
    UnpackedRecord *pUnpacked;      /* Used by VdbeSorterCompare() */
    SorterList list;                /* List of in-memory records */
    int iMemory;                    /* Offset of free space in list.aMemory */
    int nMemory;                    /* Size of list.aMemory allocation in bytes */
    u8 bUsePMA;                     /* True if one or more PMAs created */
    u8 bUseThreads;                 /* True to use background threads */
    u8 iPrev;                       /* Previous thread used to flush PMA */
    u8 nTask;                       /* Size of aTask[] array */
    u8 typeMask;
    SortSubtask aTask[1];           /* One or more subtasks */
};

struct PmaReader {
    i64 iReadOff;               /* Current read offset */
    i64 iEof;                   /* 1 byte past EOF for this PmaReader */
    int nAlloc;                 /* Bytes of space at aAlloc */
    int nKey;                   /* Number of bytes in key */
    sqlite3_file *pFd;          /* File handle we are reading from */
    u8 *aAlloc;                 /* Space for aKey if aBuffer and pMap wont work */
    u8 *aKey;                   /* Pointer to current key */
    u8 *aBuffer;                /* Current read buffer */
    int nBuffer;                /* Size of read buffer in bytes */
    u8 *aMap;                   /* Pointer to mapping of entire file */
    IncrMerger *pIncr;          /* Incremental merger */
};

struct IncrMerger {
    SortSubtask *pTask;             /* Task that owns this merger */
    MergeEngine *pMerger;           /* Merge engine thread reads data from */
    i64 iStartOff;                  /* Offset to start writing file at */
    int mxSz;                       /* Maximum bytes of data to store */
    int bEof;                       /* Set to true when merge is finished */
    int bUseThread;                 /* True to use a bg thread for this object */
    SorterFile aFile[2];            /* aFile[0] for reading, [1] for writing */
};


/*
** This object is the header on a single record while that record is being
** held in memory and prior to being written out as part of a PMA.
**
** How the linked list is connected depends on how memory is being managed
** by this module. If using a separate allocation for each in-memory record
** (VdbeSorter.list.aMemory==0), then the list is always connected using the
** SorterRecord.u.pNext pointers.
**
** Or, if using the single large allocation method (VdbeSorter.list.aMemory!=0),
** then while records are being accumulated the list is linked using the
** SorterRecord.u.iNext offset. This is because the aMemory[] array may
** be sqlite3Realloc()ed while records are being accumulated. Once the VM
** has finished passing records to the sorter, or when the in-memory buffer
** is full, the list is sorted. As part of the sorting process, it is
** converted to use the SorterRecord.u.pNext pointers. See function
** vdbeSorterSort() for details.
*/
struct SorterRecord {
    int nVal;                       /* Size of the record in bytes */
    union {
        SorterRecord *pNext;          /* Pointer to next record in list */
        int iNext;                    /* Offset within aMemory of next record */
    } u;
    /* The data for the record immediately follows this header */
};

/*
** Initialize the temporary index cursor just opened as a sorter cursor.
**
** Usually, the sorter module uses the value of (pCsr->pKeyInfo->nField)
** to determine the number of fields that should be compared from the
** records being sorted. However, if the value passed as argument nField
** is non-zero and the sorter is able to guarantee a stable sort, nField
** is used instead. This is used when sorting records for a CREATE INDEX
** statement. In this case, keys are always delivered to the sorter in
** order of the primary key, which happens to be make up the final part
** of the records being sorted. So if the sort is stable, there is never
** any reason to compare PK fields and they can be ignored for a small
** performance boost.
**
** The sorter can guarantee a stable sort when running in single-threaded
** mode, but not in multi-threaded mode.
**
** SQLITE_OK is returned if successful, or an SQLite error code otherwise.
*/
int sqlite3VdbeSorterInit(
        sqlite3 *db,                    /* Database connection (for malloc()) */
        int nField,                     /* Number of key fields in each record */
        VdbeCursor *pCsr                /* Cursor that holds the new sorter */
){
    return SQLITE_OK;
}

/*
** Reset a sorting cursor back to its original empty state.
*/
void sqlite3VdbeSorterReset(sqlite3 *db, VdbeSorter *pSorter){

}

/*
** Free any cursor components allocated by sqlite3VdbeSorterXXX routines.
*/
void sqlite3VdbeSorterClose(sqlite3 *db, VdbeCursor *pCsr){

}

/*
** Add a record to the sorter.
*/
int sqlite3VdbeSorterWrite(
        const VdbeCursor *pCsr,         /* Sorter cursor */
        Mem *pVal                       /* Memory cell containing record */
){
    int rc = SQLITE_OK;             /* Return Code */
    char* firstDataPointer = (pVal->zMalloc + 1);
    fpgaSorterInsert(firstDataPointer, (int)(*firstDataPointer)+1);

    return rc;
}


/*
** Once the sorter has been populated by calls to sqlite3VdbeSorterWrite,
** this function is called to prepare for iterating through the records
** in sorted order.
*/
int sqlite3VdbeSorterRewind(const VdbeCursor *pCsr, int *pbEof){
    int rc = SQLITE_OK;             /* Return code */

    return rc;
}

/*
** Advance to the next element in the sorter.
*/
int sqlite3VdbeSorterNext(sqlite3 *db, const VdbeCursor *pCsr, int *pbEof){
    VdbeSorter *pSorter;
    int rc;                         /* Return code */

    return rc;
}

/*
** Copy the current sorter key into the memory cell pOut.
*/
int sqlite3VdbeSorterRowkey(const VdbeCursor *pCsr, Mem *pOut){

    return SQLITE_OK;
}

/*
** Compare the key in memory cell pVal with the key that the sorter cursor
** passed as the first argument currently points to. For the purposes of
** the comparison, ignore the rowid field at the end of each record.
**
** If the sorter cursor key contains any NULL values, consider it to be
** less than pVal. Even if pVal also contains NULL values.
**
** If an error occurs, return an SQLite error code (i.e. SQLITE_NOMEM).
** Otherwise, set *pRes to a negative, zero or positive value if the
** key in pVal is smaller than, equal to or larger than the current sorter
** key.
**
** This routine forms the core of the OP_SorterCompare opcode, which in
** turn is used to verify uniqueness when constructing a UNIQUE INDEX.
*/
int sqlite3VdbeSorterCompare(
        const VdbeCursor *pCsr,         /* Sorter cursor */
        Mem *pVal,                      /* Value to compare to current sorter key */
        int nKeyCol,                    /* Compare this many columns */
        int *pRes                       /* OUT: Result of comparison */
){

    return SQLITE_OK;
}



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

        buffer[0] = (i+1) * 10;
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
    printf("\nHello, World!\n");

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
//    printf("%d%d\n", rowdata[0], rowdata[1]);
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