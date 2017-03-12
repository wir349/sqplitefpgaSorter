/*
*	CS259 Project - Linear Shift + Merge Sort for FPGAs
*	UCLA WI2017
*	Krit Sae Fang
*/
#include "fpgaSorter.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

//---------------------------------------------------------------------------------------------
// Declarations and stuff
//---------------------------------------------------------------------------------------------

// General function counters and keepers
bool isSorted;

comparePair workingArrayLinear[ListSize];
int workingArrayLinearSize;

int workingArrayLinearReadPos;

//---------------------------------------------------------------------------------------------
// "Public" functions and methods
//---------------------------------------------------------------------------------------------

/*
*	Initializer
*	Prepares values for working with this sorter
*/
void fpgaSorterInitialize() {
    // Flag set
    isSorted = false;

    // Set array counters to 0
    workingArrayLinearSize = 0;
    workingArrayLinearReadPos = 0;
}

/*
*	Adds value to be linear sorted
*	Value is kept off BRAM until ready.
*/
void fpgaSorterInsert(
        int key,
        char *data,
        int size
) {
    if (workingArrayLinearSize > ListSize) {
        // Exceeded max defined size. Return without adding.
        return;
    }
    //DEBUG
    char debugData[DataSize];
    for(int x = 0; x<DataSize; x++){
        debugData[x] = data[x];
    }
    //DEBUG
    //printf("\nAdding key %u with data value %s", key, data);
    //printf("\nCopied data value %s", debugData);

    // Copy in the key/pointer
    workingArrayLinear[workingArrayLinearSize].key = key;

    // Copy in the data
    copyArrayOuter(data, 0, workingArrayLinear[workingArrayLinearSize].data, 0, DataSize);

    //DEBUG
    //printf("\nCopied data of %s to %s", data, workingArrayLinear[workingArrayLinearSize].data);

    // Add value to local counter
    workingArrayLinearSize++;
}

/*
*	Quick reset
*	Returns counter to zero
*/
void fpgaSorterResetLinear() {
    workingArrayLinearSize = 0;
    isSorted = false;
}

/*
*	Sorts the values in our working array
*	Does not yet return anything
*/
void fpgaSorterSortLinear() {
    if (!isSorted) {
        hlsLinearSort(workingArrayLinear, workingArrayLinearSize);
        isSorted = true;
    }
}

/*
*	Gets the sorted array
*	Copies the result array into the provided array
*	Not completely safe. Array needs to be large enough to accomadate
*	Uses the struct defined in this file
*	If not yet sorted, this does nothing.
*/
void fpgaSorterGetLinearResultArray(comparePair *returnArray) {
    if (isSorted) {
        //copyArray(returnArray, 0, (char *) workingArrayLinear, 0, workingArrayLinearSize * StructSize);
        for (int i = 0; i < DataSize; i++) {
            returnArray[i] = workingArrayLinear[i];
        }
    }
}

/*
*	Gets a key from the sorted array. Returns 0 if not sorted
*/
int fpgaSorterGetLinearResultPos(int pos) {
    if (isSorted) {
        return workingArrayLinear[pos].key;
    }
    return 0;
}

/*
*	Gets a key from the sorted array. Returns 0 if not sorted or reached end
*	Returns values in sorted other each time it is called
*/
int fpgaSorterGetLinearResultNext() {
    if (isSorted && (workingArrayLinearReadPos < workingArrayLinearSize)) {
        workingArrayLinearReadPos++;

        //DEBUG
        //printf("\nReturning key %u with data value %s", workingArrayLinear[workingArrayLinearReadPos - 1].key, workingArrayLinear[workingArrayLinearReadPos - 1].data);

        return workingArrayLinear[workingArrayLinearReadPos - 1].key;
    }
    return 0;
}

/*
*	Resets the read pointer to 0.
*/
void fpgaSorterResetLinearRead() {
    workingArrayLinearReadPos = 0;
}

//---------------------------------------------------------------------------------------------
// "Private" functions and methods. Or static in this case...
//---------------------------------------------------------------------------------------------

/*
*	Main sorting algorithm
*	Maximum value that can be sorted is determined by constant ListSize
*	Sorted array overwrites given array
*/
void hlsLinearSort(
        comparePair *p,
        int pSize
) {
    #pragma HLS INTERFACE m_axi port=p bundle=gmem
    #pragma HLS INTERFACE s_axilite port=p bundle=control
    #pragma HLS INTERFACE s_axilite port=pSize bundle=control
    #pragma HLS INTERFACE s_axilite port=return bundle=control


    // Localized input array.
    comparePair pLocal[ListSize];
    //#pragma HLS DATA_PACK variable=pLocal

    // Localized result arrays
    comparePair pFinal[ListSize];
    #pragma HLS ARRAY_PARTITION variable=pFinal complete dim=1
    comparePair pFinalBuffer[ListSize];
    #pragma HLS ARRAY_PARTITION variable=pFinalBuffer complete dim=1

    //DEBUG
    //printf("\nENTERING SORT. 0 values are as follows:");
    //printf("\np[0] has key %u and value %s", p[0].key, p[0].data);
    //printf("\np[1] has key %u and value %s", p[1].key, p[1].data);

    // Copy into local BRAM.
    copyArrayBorder(p, 0, pLocal, 0, pSize * StructSize);


    // First value handling. Makes life easier with the loop
    pFinal[0] = pLocal[0];

    //DEBUG
    //printf("\nENTERING SORT LOOPS. 0 values are as follows:");
    //printf("\npLocal[0] has key %u and value %s", pLocal[0].key, pLocal[0].data);
    //printf("\npLocal[1] has key %u and value %s", pLocal[1].key, pLocal[1].data);
    //printf("\npFinal[0] has key %u and value %s", pFinal[0].key, pFinal[0].data);

    for (int i = 1; i < pSize; i++) {
        // This loop cannot be unrolled or pipelined without collision or other issues
        // Looping through the insertion

        comparePair pLocal_i=pLocal[i];

        for (int j = 1; j < ListSize; j++) {
            #pragma HLS UNROLL

            // Localize to registers
            comparePair pFinal_j = pFinal[j];
            comparePair pFinal_jn1 = pFinal[j - 1];

            // If the item in this cell is bigger and not empty...
            if (dataCompare(pFinal_j, pLocal_i) && !arrayCellIsEmpty(j, i)) {
                // Cascade move down to open up the slot we need
                if (j<(ListSize - 1)) {
                    pFinalBuffer[j + 1] = pFinal_j;
                }

                // If the item above us is smaller than the incoming value
                // Copy our value in
                if (dataCompare(pLocal_i, pFinal_jn1)) {
                    pFinalBuffer[j] = pLocal_i;
                }
            }

            // If we have an empty cell and the item above is smaller...
            if (arrayCellIsEmpty(j, i) && dataCompare(pLocal_i, pFinal_jn1)) {
                pFinalBuffer[j] = pLocal_i;
            }
        }
        // Copy values from our safety buffer back into the working array.
        copyArrayComparePair(pFinalBuffer, 0, pLocal, 0, pSize);
    }

    // Copy back into DRAM for transfer back into CPU processing
    copyArrayBorder(pLocal, 0, p, 0, pSize * StructSize);
}

/*
*	Helper function for copying arrays.
*/
void copyArray(char *source, int sourceOffset, char *destination, int destOffset, int size) {
    #pragma HLS INLINE
    for (int i = 0; i < size; i++) {
        //#pragma HLS UNROLL
        destination[i + destOffset] = source[i + sourceOffset];

        //DEBUG
        //printf("\nCopying char array, size %u", size);
        //printf("\nCopying char array, pos %i from %c to %c", i, source[i + sourceOffset],destination[i + destOffset]);
    }
}

void copyArrayComparePair(comparePair *source, int sourceOffset, comparePair *destination, int destOffset, int size) {
    #pragma HLS INLINE
    //copyArray((char *) source, sourceOffset, (char*) destination, destOffset, size);
    for (int i = 0; i < size; i++) {
        //#pragma HLS UNROLL
        //DEBUG
        //printf("\nCopying comparePair array, size %u", size);

        //destination[i + destOffset].key = source[i + sourceOffset].key;
        //copyArray(source[i + sourceOffset].data, 0, destination[i + destOffset].data, 0, DataSize);

        destination[i + destOffset] = source[i + sourceOffset];

        //DEBUG
        //printf("\nCopying comparePair array, pos %i from %s to %s", i, source[i + sourceOffset].data,destination[i + destOffset].data);
    }
}

void copyArrayBorder(void * source, int sourceOffset, void * destination, int destOffset, int size){
    #pragma HLS INLINE
    memcpy(destination + destOffset, source + sourceOffset, size);
}

void copyArrayOuter(void * source, int sourceOffset, void * destination, int destOffset, int size){
    memcpy(destination + destOffset, source + sourceOffset, size);
}

/*
*	Helper function for comparing values in the struct
*	Bitwise, byte by byte via char
*/
bool dataCompare(comparePair x, comparePair y) {
    for (int i = 0; i < DataSize; i++) {
        if (x.data[i] > y.data[i]) {
            return true;
        }
    }
    return false;
}

/*
*	Helper function for checking for empty cell.
*	Makes life easier since I don't have to do multiple nests after nests
*/
bool arrayCellIsEmpty(int i, int j) {
    if (i >= j) {
        return true;
    }
    return false;
}

/*
*	Merge sort implementation for FPGAs
*	INCOMPLETE CODE
*/
/*
static void hlsMergeSort(
        char *p1,
        int p1Size,
        char *p2,
        int p2Size,
        char *pFinal
) {
    //#pragma HLS

    int currentFinalLocalSize;
    int currentFinalSize;
    int p1LocalSize;
    int p2LocalSize;
    int p1Remain;
    int p2Remain;
    int p1Moves;
    int p2Moves;

    // Localized input array.
    comparePair p1Local[InputBufferSize];
    comparePair p2Local[InputBufferSize];
    //comparePair p1LocalBuffer[InputBufferMin];
    //comparePair p2LocalBugger[InputBufferMin];

    // Localized result arrays
    comparePair pFinalLocal[ListSize];

    // Initiate remaining counters
    p1Remain = p1Size;
    p2Remain = p2Size;

    // Initiate merge counters
    p1LocalSize = 0;
    p2LocalSize = 0;
    p1Moves = 0;
    p2Moves = 0;
    currentFinalLocalSize = 0;
    currentFinalSize = 0;

    // Loop for merge sorting
    while (p1Remain > 0 || p2Remain > 0) {

        // Move local final list to DRAM if filled
        if (currentFinalLocalSize > ListSize - 1) {
            copyArray(pFinal, (currentFinalSize * StructSize), (char *) pFinalLocal, 0, ListSize * StructSize);
            currentFinalSize = currentFinalSize + currentFinalLocalSize;
        }

        // Fill local p1 if empty
        if (p1Remain > 0 && p1LocalSize == 0) {
            if (p1Remain < InputBufferSize) {
                copyArray((char *) p1Local, 0, p1 ,(p1Moves * DataSize * ListSize), InputBufferSize * StructSize);
                p1Remain = p1Remain - InputBufferSize;
                p1LocalSize = InputBufferSize;
            } else {
                copyArray((char *) p1Local, 0, p1, (p1Moves * DataSize * ListSize), p1Remain * StructSize);
                p1Remain = 0;
                p1LocalSize = p1Remain;
            }
            p1Moves++;
        }

        // Fill local p2 if empty
        if (p2Remain > 0 && p2LocalSize == 0) {
            if (p2Remain < InputBufferSize) {
                copyArray((char *) p1Local, 0, p2, (p2Moves * DataSize * ListSize), InputBufferSize * StructSize);
                p2Remain = p2Remain - InputBufferSize;
                p2LocalSize = InputBufferSize;
            } else {
                copyArray((char *) p1Local, 0, p2 ,(p2Moves * DataSize * ListSize), p1Remain * StructSize);
                p2Remain = 0;
                p2LocalSize = p2Remain;
            }
            p2Moves++;
        }
    }

    // Copy into local BRAM.
    copyArray((char *) p1Local, 0, p1, 0, p1Size * StructSize);
    copyArray((char *) p2Local, 0, p2, 0, p2Size * StructSize);


}*/
