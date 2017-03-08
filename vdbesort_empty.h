//
// Created by Waleed Rahman on 3/7/17.
//

#ifndef FPGASORTER_FPGASORTER_H
#define FPGASORTER_FPGASORTER_H

#endif //FPGASORTER_FPGASORTER_H

/*
** CAPI3REF: Result Codes
** KEYWORDS: {result code definitions}
**
** Many SQLite functions return an integer result code from the set shown
** here in order to indicate success or failure.
**
** New error codes may be added in future versions of SQLite.
**
** See also: [extended result code definitions]
*/
#define SQLITE_OK           0   /* Successful result */
/* beginning-of-error-codes */
#define SQLITE_ERROR        1   /* SQL error or missing database */
#define SQLITE_INTERNAL     2   /* Internal logic error in SQLite */
#define SQLITE_PERM         3   /* Access permission denied */
#define SQLITE_ABORT        4   /* Callback routine requested an abort */
#define SQLITE_BUSY         5   /* The database file is locked */
#define SQLITE_LOCKED       6   /* A table in the database is locked */
#define SQLITE_NOMEM        7   /* A malloc() failed */
#define SQLITE_READONLY     8   /* Attempt to write a readonly database */
#define SQLITE_INTERRUPT    9   /* Operation terminated by sqlite3_interrupt()*/
#define SQLITE_IOERR       10   /* Some kind of disk I/O error occurred */
#define SQLITE_CORRUPT     11   /* The database disk image is malformed */
#define SQLITE_NOTFOUND    12   /* Unknown opcode in sqlite3_file_control() */
#define SQLITE_FULL        13   /* Insertion failed because database is full */
#define SQLITE_CANTOPEN    14   /* Unable to open the database file */
#define SQLITE_PROTOCOL    15   /* Database lock protocol error */
#define SQLITE_EMPTY       16   /* Database is empty */
#define SQLITE_SCHEMA      17   /* The database schema changed */
#define SQLITE_TOOBIG      18   /* String or BLOB exceeds size limit */
#define SQLITE_CONSTRAINT  19   /* Abort due to constraint violation */
#define SQLITE_MISMATCH    20   /* Data type mismatch */
#define SQLITE_MISUSE      21   /* Library used incorrectly */
#define SQLITE_NOLFS       22   /* Uses OS features not supported on host */
#define SQLITE_AUTH        23   /* Authorization denied */
#define SQLITE_FORMAT      24   /* Auxiliary database format error */
#define SQLITE_RANGE       25   /* 2nd parameter to sqlite3_bind out of range */
#define SQLITE_NOTADB      26   /* File opened that is not a database file */
#define SQLITE_NOTICE      27   /* Notifications from sqlite3_log() */
#define SQLITE_WARNING     28   /* Warnings from sqlite3_log() */
#define SQLITE_ROW         100  /* sqlite3_step() has another row ready */
#define SQLITE_DONE        101  /* sqlite3_step() has finished executing */
/* end-of-error-codes */

/*
** Boolean values
*/
typedef unsigned Bool;

/* Opaque type used by code in vdbesort.c */
typedef struct VdbeSorter VdbeSorter;

/*
** CAPI3REF: Database Connection Handle
** KEYWORDS: {database connection} {database connections}
**
** Each open SQLite database is represented by a pointer to an instance of
** the opaque structure named "sqlite3".  It is useful to think of an sqlite3
** pointer as an object.  The [sqlite3_open()], [sqlite3_open16()], and
** [sqlite3_open_v2()] interfaces are its constructors, and [sqlite3_close()]
** and [sqlite3_close_v2()] are its destructors.  There are many other
** interfaces (such as
** [sqlite3_prepare_v2()], [sqlite3_create_function()], and
** [sqlite3_busy_timeout()] to name but three) that are methods on an
** sqlite3 object.
*/
typedef struct sqlite3 sqlite3;

/*
** CAPI3REF: 64-Bit Integer Types
** KEYWORDS: sqlite_int64 sqlite_uint64
**
** Because there is no cross-platform way to specify 64-bit integer types
** SQLite includes typedefs for 64-bit signed and unsigned integers.
**
** The sqlite3_int64 and sqlite3_uint64 are the preferred type definitions.
** The sqlite_int64 and sqlite_uint64 types are supported for backwards
** compatibility only.
**
** ^The sqlite3_int64 and sqlite_int64 types can store integer values
** between -9223372036854775808 and +9223372036854775807 inclusive.  ^The
** sqlite3_uint64 and sqlite_uint64 types can store integer values
** between 0 and +18446744073709551615 inclusive.
*/
#ifdef SQLITE_INT64_TYPE
typedef SQLITE_INT64_TYPE sqlite_int64;
# ifdef SQLITE_UINT64_TYPE
    typedef SQLITE_UINT64_TYPE sqlite_uint64;
# else
    typedef unsigned SQLITE_INT64_TYPE sqlite_uint64;
# endif
#elif defined(_MSC_VER) || defined(__BORLANDC__)
typedef __int64 sqlite_int64;
  typedef unsigned __int64 sqlite_uint64;
#else
typedef long long int sqlite_int64;
typedef unsigned long long int sqlite_uint64;
#endif
typedef sqlite_int64 sqlite3_int64;
typedef sqlite_uint64 sqlite3_uint64;


/*
** CAPI3REF: OS Interface Open File Handle
**
** An [sqlite3_file] object represents an open file in the
** [sqlite3_vfs | OS interface layer].  Individual OS interface
** implementations will
** want to subclass this object by appending additional fields
** for their own use.  The pMethods entry is a pointer to an
** [sqlite3_io_methods] object that defines methods for performing
** I/O operations on the open file.
*/
typedef struct sqlite3_file sqlite3_file;
struct sqlite3_file {
    const struct sqlite3_io_methods *pMethods;  /* Methods for an open file */
};

/*
** Integers of known sizes.  These typedefs might change for architectures
** where the sizes very.  Preprocessor macros are available so that the
** types can be conveniently redefined at compile-type.  Like this:
**
**         cc '-DUINTPTR_TYPE=long long int' ...
*/
#ifndef UINT32_TYPE
# ifdef HAVE_UINT32_T
#  define UINT32_TYPE uint32_t
# else
#  define UINT32_TYPE unsigned int
# endif
#endif
#ifndef UINT16_TYPE
# ifdef HAVE_UINT16_T
#  define UINT16_TYPE uint16_t
# else
#  define UINT16_TYPE unsigned short int
# endif
#endif
#ifndef INT16_TYPE
# ifdef HAVE_INT16_T
#  define INT16_TYPE int16_t
# else
#  define INT16_TYPE short int
# endif
#endif
#ifndef UINT8_TYPE
# ifdef HAVE_UINT8_T
#  define UINT8_TYPE uint8_t
# else
#  define UINT8_TYPE unsigned char
# endif
#endif
#ifndef INT8_TYPE
# ifdef HAVE_INT8_T
#  define INT8_TYPE int8_t
# else
#  define INT8_TYPE signed char
# endif
#endif
#ifndef LONGDOUBLE_TYPE
# define LONGDOUBLE_TYPE long double
#endif
typedef sqlite_int64 i64;          /* 8-byte signed integer */
typedef sqlite_uint64 u64;         /* 8-byte unsigned integer */
typedef UINT32_TYPE u32;           /* 4-byte unsigned integer */
typedef UINT16_TYPE u16;           /* 2-byte unsigned integer */
typedef INT16_TYPE i16;            /* 2-byte signed integer */
typedef UINT8_TYPE u8;             /* 1-byte unsigned integer */
typedef INT8_TYPE i8;              /* 1-byte signed integer */

/*
** Forward references to structures
*/
//typedef struct AggInfo AggInfo;
//typedef struct AuthContext AuthContext;
//typedef struct AutoincInfo AutoincInfo;
//typedef struct Bitvec Bitvec;
//typedef struct CollSeq CollSeq;
//typedef struct Column Column;
//typedef struct Db Db;
//typedef struct Schema Schema;
//typedef struct Expr Expr;
//typedef struct ExprList ExprList;
//typedef struct ExprSpan ExprSpan;
//typedef struct FKey FKey;
//typedef struct FuncDestructor FuncDestructor;
typedef struct FuncDef FuncDef;
//typedef struct FuncDefHash FuncDefHash;
//typedef struct IdList IdList;
//typedef struct Index Index;
//typedef struct IndexSample IndexSample;
//typedef struct KeyClass KeyClass;
typedef struct KeyInfo KeyInfo;
//typedef struct Lookaside Lookaside;
//typedef struct LookasideSlot LookasideSlot;
//typedef struct Module Module;
//typedef struct NameContext NameContext;
//typedef struct Parse Parse;
//typedef struct PreUpdate PreUpdate;
//typedef struct PrintfArguments PrintfArguments;
typedef struct RowSet RowSet;
//typedef struct Savepoint Savepoint;
//typedef struct Select Select;
typedef struct SQLiteThread SQLiteThread;
//typedef struct SelectDest SelectDest;
//typedef struct SrcList SrcList;
//typedef struct StrAccum StrAccum;
//typedef struct Table Table;
//typedef struct TableLock TableLock;
//typedef struct Token Token;
//typedef struct TreeView TreeView;
//typedef struct Trigger Trigger;
//typedef struct TriggerPrg TriggerPrg;
//typedef struct TriggerStep TriggerStep;
typedef struct UnpackedRecord UnpackedRecord;
//typedef struct VTable VTable;
//typedef struct VtabCtx VtabCtx;
//typedef struct Walker Walker;
//typedef struct WhereInfo WhereInfo;
//typedef struct With With;

/*
** Forward declarations of structure
*/
typedef struct Btree Btree;
typedef struct BtCursor BtCursor;
typedef struct BtShared BtShared;
typedef struct BtreePayload BtreePayload;

/* Forward declarations */
typedef struct MemPage MemPage;
typedef struct BtLock BtLock;
typedef struct CellInfo CellInfo;

/*
** The type used to represent a page number.  The first page in a file
** is called page 1.  0 is used to represent "not a page".
*/
typedef u32 Pgno;


/*
** Structures used by the virtual table interface
*/
typedef struct sqlite3_vtab sqlite3_vtab;
typedef struct sqlite3_index_info sqlite3_index_info;
typedef struct sqlite3_vtab_cursor sqlite3_vtab_cursor;
typedef struct sqlite3_module sqlite3_module;

/*
** The names of the following types declared in vdbeInt.h are required
** for the VdbeOp definition.
*/
typedef struct Mem Mem;
typedef struct SubProgram SubProgram;

/*
** SQL is translated into a sequence of instructions to be
** executed by a virtual machine.  Each instruction is an instance
** of the following structure.
*/
typedef struct VdbeOp Op;


/*
** A single VDBE is an opaque structure named "Vdbe".  Only routines
** in the source file sqliteVdbe.c are allowed to see the insides
** of this structure.
*/
typedef struct Vdbe Vdbe;

/* Elements of the linked list at Vdbe.pAuxData */
typedef struct AuxData AuxData;



/*
** CAPI3REF: Virtual Table Cursor Object
** KEYWORDS: sqlite3_vtab_cursor {virtual table cursor}
**
** Every [virtual table module] implementation uses a subclass of the
** following structure to describe cursors that point into the
** [virtual table] and are used
** to loop through the virtual table.  Cursors are created using the
** [sqlite3_module.xOpen | xOpen] method of the module and are destroyed
** by the [sqlite3_module.xClose | xClose] method.  Cursors are used
** by the [xFilter], [xNext], [xEof], [xColumn], and [xRowid] methods
** of the module.  Each module implementation will define
** the content of a cursor structure to suit its own needs.
**
** This superclass exists in order to define fields of the cursor that
** are common to all implementations.
*/
struct sqlite3_vtab_cursor {
    sqlite3_vtab *pVtab;      /* Virtual table of this cursor */
    /* Virtual table implementations will typically add additional fields */
};

/*
** A linked list of the following structures is stored at BtShared.pLock.
** Locks are added (or upgraded from READ_LOCK to WRITE_LOCK) when a cursor
** is opened on the table with root page BtShared.iTable. Locks are removed
** from this list when a transaction is committed or rolled back, or when
** a btree handle is closed.
*/
struct BtLock {
    Btree *pBtree;        /* Btree handle holding this lock */
    Pgno iTable;          /* Root page of table */
    u8 eLock;             /* READ_LOCK or WRITE_LOCK */
    BtLock *pNext;        /* Next in BtShared.pLock list */
};

/*
** A VdbeCursor is an superclass (a wrapper) for various cursor objects:
**
**      * A b-tree cursor
**          -  In the main database or in an ephemeral database
**          -  On either an index or a table
**      * A sorter
**      * A virtual table
**      * A one-row "pseudotable" stored in a single register
*/
typedef struct VdbeCursor VdbeCursor;
struct VdbeCursor {
    u8 eCurType;            /* One of the CURTYPE_* values above */
    i8 iDb;                 /* Index of cursor database in db->aDb[] (or -1) */
    u8 nullRow;             /* True if pointing to a row with no data */
    u8 deferredMoveto;      /* A call to sqlite3BtreeMoveto() is needed */
    u8 isTable;             /* True for rowid tables.  False for indexes */
#ifdef SQLITE_DEBUG
    u8 seekOp;              /* Most recent seek operation on this cursor */
  u8 wrFlag;              /* The wrFlag argument to sqlite3BtreeCursor() */
#endif
    Bool isEphemeral:1;     /* True for an ephemeral table */
    Bool useRandomRowid:1;  /* Generate new record numbers semi-randomly */
    Bool isOrdered:1;       /* True if the table is not BTREE_UNORDERED */
    Btree *pBtx;            /* Separate file holding temporary table */
    i64 seqCount;           /* Sequence counter */
    int *aAltMap;           /* Mapping from table to index column numbers */

    /* Cached OP_Column parse information is only valid if cacheStatus matches
    ** Vdbe.cacheCtr.  Vdbe.cacheCtr will never take on the value of
    ** CACHE_STALE (0) and so setting cacheStatus=CACHE_STALE guarantees that
    ** the cache is out of date. */
    u32 cacheStatus;        /* Cache is valid if this matches Vdbe.cacheCtr */
    int seekResult;         /* Result of previous sqlite3BtreeMoveto() or 0
                          ** if there have been no prior seeks on the cursor. */
    /* NB: seekResult does not distinguish between "no seeks have ever occurred
    ** on this cursor" and "the most recent seek was an exact match". */

    /* When a new VdbeCursor is allocated, only the fields above are zeroed.
    ** The fields that follow are uninitialized, and must be individually
    ** initialized prior to first use. */
    VdbeCursor *pAltCursor; /* Associated index cursor from which to read */
    union {
        BtCursor *pCursor;          /* CURTYPE_BTREE.  Btree cursor */
        sqlite3_vtab_cursor *pVCur; /* CURTYPE_VTAB.   Vtab cursor */
        int pseudoTableReg;         /* CURTYPE_PSEUDO. Reg holding content. */
        VdbeSorter *pSorter;        /* CURTYPE_SORTER. Sorter object */
    } uc;
    KeyInfo *pKeyInfo;      /* Info about index keys needed by index cursors */
    u32 iHdrOffset;         /* Offset to next unparsed byte of the header */
    Pgno pgnoRoot;          /* Root page of the open btree cursor */
    i16 nField;             /* Number of fields in the header */
    u16 nHdrParsed;         /* Number of header fields parsed so far */
    i64 movetoTarget;       /* Argument to the deferred sqlite3BtreeMoveto() */
    u32 *aOffset;           /* Pointer to aType[nField] */
    const u8 *aRow;         /* Data for the current row, if all on one page */
    u32 payloadSize;        /* Total number of bytes in the record */
    u32 szRow;              /* Byte available in aRow */
#ifdef SQLITE_ENABLE_COLUMN_USED_MASK
    u64 maskUsed;           /* Mask of columns used by this cursor */
#endif

    /* 2*nField extra array elements allocated for aType[], beyond the one
    ** static element declared in the structure.  nField total array slots for
    ** aType[] and nField+1 array slots for aOffset[] */
    u32 aType[1];           /* Type values record decode.  MUST BE LAST */
};

/* A Btree handle
**
** A database connection contains a pointer to an instance of
** this object for every database file that it has open.  This structure
** is opaque to the database connection.  The database connection cannot
** see the internals of this structure and only deals with pointers to
** this structure.
**
** For some database files, the same underlying database cache might be
** shared between multiple connections.  In that case, each connection
** has it own instance of this object.  But each instance of this object
** points to the same BtShared object.  The database cache and the
** schema associated with the database file are all contained within
** the BtShared object.
**
** All fields in this structure are accessed under sqlite3.mutex.
** The pBt pointer itself may not be changed while there exists cursors
** in the referenced BtShared that point back to this Btree since those
** cursors have to go through this Btree to find their BtShared and
** they often do so without holding sqlite3.mutex.
*/
struct Btree {
    sqlite3 *db;       /* The database connection holding this btree */
    BtShared *pBt;     /* Sharable content of this btree */
    u8 inTrans;        /* TRANS_NONE, TRANS_READ or TRANS_WRITE */
    u8 sharable;       /* True if we can share pBt with another db */
    u8 locked;         /* True if db currently has pBt locked */
    u8 hasIncrblobCur; /* True if there are one or more Incrblob cursors */
    int wantToLock;    /* Number of nested calls to sqlite3BtreeEnter() */
    int nBackup;       /* Number of backup operations reading this btree */
    u32 iDataVersion;  /* Combines with pBt->pPager->iDataVersion */
    Btree *pNext;      /* List of other sharable Btrees from the same db */
    Btree *pPrev;      /* Back pointer of the same list */
#ifndef SQLITE_OMIT_SHARED_CACHE
    BtLock lock;       /* Object used to lock page 1 */
#endif
};


/*
** When a sub-program is executed (OP_Program), a structure of this type
** is allocated to store the current value of the program counter, as
** well as the current memory cell array and various other frame specific
** values stored in the Vdbe struct. When the sub-program is finished,
** these values are copied back to the Vdbe from the VdbeFrame structure,
** restoring the state of the VM to as it was before the sub-program
** began executing.
**
** The memory for a VdbeFrame object is allocated and managed by a memory
** cell in the parent (calling) frame. When the memory cell is deleted or
** overwritten, the VdbeFrame object is not freed immediately. Instead, it
** is linked into the Vdbe.pDelFrame list. The contents of the Vdbe.pDelFrame
** list is deleted when the VM is reset in VdbeHalt(). The reason for doing
** this instead of deleting the VdbeFrame immediately is to avoid recursive
** calls to sqlite3VdbeMemRelease() when the memory cells belonging to the
** child frame are released.
**
** The currently executing frame is stored in Vdbe.pFrame. Vdbe.pFrame is
** set to NULL if the currently executing frame is the main program.
*/
typedef struct VdbeFrame VdbeFrame;
struct VdbeFrame {
    Vdbe *v;                /* VM this frame belongs to */
    VdbeFrame *pParent;     /* Parent of this frame, or NULL if parent is main */
    Op *aOp;                /* Program instructions for parent frame */
    i64 *anExec;            /* Event counters from parent frame */
    Mem *aMem;              /* Array of memory cells for parent frame */
    VdbeCursor **apCsr;     /* Array of Vdbe cursors for parent frame */
    void *token;            /* Copy of SubProgram.token */
    i64 lastRowid;          /* Last insert rowid (sqlite3.lastRowid) */
    AuxData *pAuxData;      /* Linked list of auxdata allocations */
    int nCursor;            /* Number of entries in apCsr */
    int pc;                 /* Program Counter in parent (calling) frame */
    int nOp;                /* Size of aOp array */
    int nMem;               /* Number of entries in aMem */
    int nChildMem;          /* Number of memory cells for child frame */
    int nChildCsr;          /* Number of cursors for child frame */
    int nChange;            /* Statement changes (Vdbe.nChange)     */
    int nDbChange;          /* Value of db->nChange */
};


/*
** Internally, the vdbe manipulates nearly all SQL values as Mem
** structures. Each Mem struct may cache multiple representations (string,
** integer etc.) of the same value.
*/
struct Mem {
    union MemValue {
        double r;           /* Real value used when MEM_Real is set in flags */
        i64 i;              /* Integer value used when MEM_Int is set in flags */
        int nZero;          /* Used when bit MEM_Zero is set in flags */
        FuncDef *pDef;      /* Used only when flags==MEM_Agg */
        RowSet *pRowSet;    /* Used only when flags==MEM_RowSet */
        VdbeFrame *pFrame;  /* Used when flags==MEM_Frame */
    } u;
    u16 flags;          /* Some combination of MEM_Null, MEM_Str, MEM_Dyn, etc. */
    u8  enc;            /* SQLITE_UTF8, SQLITE_UTF16BE, SQLITE_UTF16LE */
    u8  eSubtype;       /* Subtype for this value */
    int n;              /* Number of characters in string value, excluding '\0' */
    char *z;            /* String or BLOB value */
    /* ShallowCopy only needs to copy the information above */
    char *zMalloc;      /* Space to hold MEM_Str or MEM_Blob if szMalloc>0 */
    int szMalloc;       /* Size of the zMalloc allocation */
    u32 uTemp;          /* Transient storage for serial_type in OP_MakeRecord */
    sqlite3 *db;        /* The associated database connection */
    void (*xDel)(void*);/* Destructor for Mem.z - only valid if MEM_Dyn */
#ifdef SQLITE_DEBUG
    Mem *pScopyFrom;    /* This Mem is a shallow copy of pScopyFrom */
  void *pFiller;      /* So that sizeof(Mem) is a multiple of 8 */
#endif
};

int sqlite3VdbeSorterInit(sqlite3 *, int, VdbeCursor *);
void sqlite3VdbeSorterReset(sqlite3 *, VdbeSorter *);
void sqlite3VdbeSorterClose(sqlite3 *, VdbeCursor *);
int sqlite3VdbeSorterRowkey(const VdbeCursor *, Mem *);
int sqlite3VdbeSorterNext(sqlite3 *, const VdbeCursor *, int *);
int sqlite3VdbeSorterRewind(const VdbeCursor *, int *);
int sqlite3VdbeSorterWrite(const VdbeCursor *, Mem *);
int sqlite3VdbeSorterCompare(const VdbeCursor *, Mem *, int, int *);