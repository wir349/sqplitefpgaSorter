//
// Created by Krittercon on 3/12/2017.
//
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "fpgaSorter.h"

bool initialized;
bool isSorted;
ComparePair workingArrayLinear[ListSize];
int workingArrayLinearSize;
int workingArrayLinearReadPos;

void kernel(ComparePair * a, int16_t p, int16_t q) {
    int16_t d = 1 << (p-q);

    for(int16_t i=0;i<ListSize;i++) {
#pragma HLS UNROLL
        bool up = ((i >> p) & 2) == 0;
        int16_t pos2 = (i | d);
        ComparePair pairA = a[i];
        ComparePair pairB = a[pos2];

        if ((i & d) == 0 && (pairA.data > pairB.data) == up) {
            a[i] = pairB;
            a[pos2] = pairA;
        }
    }
}

/*
 *  Initializer
*/
void fpgaSorterInitialize(int sortSize) {
    if (!initialized) {
        isSorted = false;
        workingArrayLinearSize = 0;
        workingArrayLinearReadPos = 0;

        initialized = true;
    }
}

/*
 *  Adds an entry
 */
void fpgaSorterInsert(int key, char *data, int size) {

    if (!initialized) {
        return;
    }

    //DEBUG
    key = workingArrayLinearSize;

    workingArrayLinear[workingArrayLinearSize].key = key;
    memcpy(&workingArrayLinear[workingArrayLinearSize].data, data, DataSize);

    //DEBUG
    workingArrayLinear[workingArrayLinearSize].data = rand();

    printf("\nInserting key %hd with value %lld", workingArrayLinear[workingArrayLinearSize].key,
           workingArrayLinear[workingArrayLinearSize].data);

    workingArrayLinearSize++;
}

void fpgaSorterSortLinear() {
    if (!isSorted) {
        hlsLinearSort(workingArrayLinear, workingArrayLinearSize);
        isSorted = true;
    }
}

int fpgaSorterGetLinearResultNext() {
    if (isSorted && (workingArrayLinearReadPos < workingArrayLinearSize)) {
        workingArrayLinearReadPos++;

        //DEBUG
        printf("\nReturning key %hd with value %lld", workingArrayLinear[workingArrayLinearReadPos - 1].key,
               workingArrayLinear[workingArrayLinearReadPos - 1].data);

        return workingArrayLinear[workingArrayLinearReadPos - 1].key;
    }

    return 0;
}

/*
 * hlsLinearSort
 * Sorts in groups of ListSize
 * Output should be read in batches of ListSize
 */
void hlsLinearSort(ComparePair *workingArray, int16_t inputSize) {
#pragma HLS INTERFACE m_axi port=workingArray bundle=gmem
#pragma HLS INTERFACE s_axilite port=workingArray bundle=control
#pragma HLS INTERFACE s_axilite port=inputSize bundle=control
#pragma HLS INTERFACE s_axilite port=return bundle=control

    // Localized arrays
    ComparePair localFinalArray[ListSize];
#pragma HLS ARRAY_PARTITION variable=localFinalArray complete dim=1

    // Copy into local array
    for (int16_t workingArrayPosA = 0; workingArrayPosA < ListSize; workingArrayPosA++) {
//#pragma HLS PIPELINE
        localFinalArray[workingArrayPosA] = workingArray[workingArrayPosA];
    }

    printf("\nStarting sorting loop");
    for(int i=0;i<9;i++) {
        for(int j=0;j<=i;j++) {
            kernel(localFinalArray, i, j);
        }
    }
    printf("\nSorting loop completed");

    // Copy back into passed in array
    for (int16_t workingArrayPosB = 0; workingArrayPosB < ListSize; workingArrayPosB++) {
//#pragma HLS PIPELINE
        workingArray[workingArrayPosB] = localFinalArray[workingArrayPosB];
    }

    //}
}
