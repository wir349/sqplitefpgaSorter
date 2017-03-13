/*
*	CS259 Project - Linear Shift + Merge Sort for FPGAs
*	UCLA WI2017
*	Krit Sae Fang
*/
#include "fpgaSorter.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

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
    key = workingArrayLinearSize;

    // Copy in the key/pointer
    workingArrayLinear[workingArrayLinearSize].key = key;

    // Copy in the data
    copyArrayOuter(data, 0, &workingArrayLinear[workingArrayLinearSize].data, 0, DataSize);

    //DEBUG
    workingArrayLinear[workingArrayLinearSize].data = rand();
    printf("\nAdding key %i with data value %i", key, workingArrayLinear[workingArrayLinearSize].data);

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
        printf("\nReturning key %i with data value %i", workingArrayLinear[workingArrayLinearReadPos - 1].key, workingArrayLinear[workingArrayLinearReadPos - 1].data);

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
        uint16_t pSize
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

    // Copy into local BRAM.
    copyArrayBorder(p, 0, pLocal, 0, pSize * StructSize);


    // First value handling. Makes life easier with the loop
    pFinal[0] = pLocal[0];
    printf("\nInserting to sort key %i with data value %i", pLocal[0].key, pLocal[0].data);

    for (int i = 1; i < pSize; i++) {
        // This loop cannot be unrolled or pipelined without collision or other issues
        // Looping through the insertion

        comparePair pLocal_i = pLocal[i];
        printf("\nInserting to sort key %i with data value %i", pLocal[i].key, pLocal[i].data);
        printf("\nInserting to sort key %i with data value %i", pLocal_i.key, pLocal_i.data);

        for (int j = 1; j < ListSize; j++) {
            #pragma HLS UNROLL

            // Localize to registers
            comparePair pFinal_jn1 = pFinal[j - 1];
            comparePair pFinal_j = pFinal[j];
            comparePair pFinalBuffer_j;

            if (arrayCellIsEmpty(j, i) && !arrayCellIsEmpty((j-1), i)){
                // This is an empty cell, the one above isn't
                printf("\nDetected empty cell and filled cell above");
                printf("\nInserting to sort key %i with data value %i", pFinal_jn1.key, pFinal_jn1.data);
                printf("\nInserting to sort key %i with data value %i", pLocal_i.key, pLocal_i.data);
                if(dataCompare(pFinal_jn1.data, pLocal_i.data)){
                    // If the cell above has a larger value than the one coming in, we're receiving it
                    pFinalBuffer_j = pFinal_jn1;

                } else {
                    // Else, the cell above is smaller, and the value here sould be the incoming value
                    pFinalBuffer_j = pLocal_i;

                }
            } else if(!arrayCellIsEmpty((j-1), i)) {
                // This cell is occupied (Cell above must be occupied as well
                printf("\nDetected filled cell");
                if(dataCompare(pFinal_jn1.data, pLocal_i.data)){
                    // If the cell above has a larger value than the one coming in, we're receiving it
                    pFinalBuffer_j = pFinal_jn1;

                } else if(dataCompare(pFinal_j.data, pLocal_i.data)) {
                    // If the cell above is not larger, and we have a larger value, it should go here
                    pFinalBuffer_j = pLocal_i;

                }
            }

            pFinal[j] = pFinalBuffer_j;

        }
    }

    // Copy back into DRAM for transfer back into CPU processing
    copyArrayBorder(pLocal, 0, p, 0, pSize * StructSize);
}

/*
*	Helper function for copying arrays.
*/
void copyArray(char *source, uint8_t sourceOffset, char *destination, uint8_t destOffset, uint16_t size) {
    #pragma HLS INLINE
    for (int16_t i = 0; i < size; i++) {
        //#pragma HLS UNROLL
        destination[i + destOffset] = source[i + sourceOffset];

        //DEBUG
        //printf("\nCopying char array, size %u", size);
        //printf("\nCopying char array, pos %i from %c to %c", i, source[i + sourceOffset],destination[i + destOffset]);
    }
}

void copyArrayComparePair(comparePair *source, uint8_t sourceOffset, comparePair *destination, uint8_t destOffset, uint16_t size) {
    #pragma HLS INLINE
    //copyArray((char *) source, sourceOffset, (char*) destination, destOffset, size);
    for (int16_t i = 0; i < size; i++) {
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
//bool dataCompare(comparePair x, comparePair y) {
//    for (int i = 0; i < DataSize; i++) {
//        if (x.data[i] > y.data[i]) {
//            return true;
//        }
//    }
//    return false;
//}
bool dataCompare(int64_t a, int64_t b) {
    printf("\nCaparing values,x = %i, y = %i", a, b);

    if (a > b) {
        printf("\nReturned true");
        return true;

    }
    printf("\nReturned true");
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
