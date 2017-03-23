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
comparePair workingArrayLinear[ListSize * 2];
//comparePair * workingArrayLinear;
int maxSize;
int workingArrayLinearSize;
int workingArrayLinearReadPos;

/*
 *  Initializer
*/
void fpgaSorterInitialize(int sortSize) {
    if (!initialized) {
        isSorted = false;
        workingArrayLinearSize = 0;
        workingArrayLinearReadPos = 0;

        maxSize = sortSize;
        //DISABLED UNTIL MALLOC IS SORTED
        //workingArrayLinear = (comparePair*) malloc(sortSize * ComparePairSize);

        initialized = true;
    }
}

/*
 *  Adds an entry
 */
void fpgaSorterInsert(int key, char *data, int size) {

    if (!initialized) {
        //printf("\nCannot sort: Sort size not initialized");
        return;
    }

    //DEBUG
    key = workingArrayLinearSize;

    workingArrayLinear[workingArrayLinearSize].key = key;
    memcpy(&workingArrayLinear[workingArrayLinearSize].data, data, DataSize);

    //DEBUG
    workingArrayLinear[workingArrayLinearSize].data = rand();

    workingArrayLinearSize++;
}

void fpgaSorterSortLinear() {
    if (!isSorted) {
        int remainingToSort = workingArrayLinearSize;
        int loopIteration = 0;
        comparePair arrayToSort[ListSize];

        while (remainingToSort > 0) {
            if (remainingToSort > ListSize){
                for (int workingArrayPos1 = 0; workingArrayPos1 < ListSize; workingArrayPos1++) {
                    arrayToSort[workingArrayPos1] = workingArrayLinear[workingArrayPos1 + (loopIteration * ListSize)];
                }
                hlsLinearSort(arrayToSort,ListSize);
                for (int workingArrayPos2 = 0; workingArrayPos2 < ListSize; workingArrayPos2++) {
                    workingArrayLinear[workingArrayPos2 + (loopIteration * ListSize)] = arrayToSort[workingArrayPos2];
                }
                remainingToSort -= ListSize;
            } else {
                for (int workingArrayPos3 = 0; workingArrayPos3 < remainingToSort; workingArrayPos3++) {
                    arrayToSort[workingArrayPos3] = workingArrayLinear[workingArrayPos3 + (loopIteration * ListSize)];
                }
                hlsLinearSort(arrayToSort,remainingToSort);
                for (int workingArrayPos4 = 0; workingArrayPos4 < remainingToSort; workingArrayPos4++) {
                    workingArrayLinear[workingArrayPos4 + (loopIteration * ListSize)] = arrayToSort[workingArrayPos4];
                }
                remainingToSort = 0;
            }
            loopIteration++;
        }


        //hlsLinearSort(workingArrayLinear, workingArrayLinearSize);
        isSorted = true;
    }
}

int fpgaSorterGetLinearResultNext() {
    if (isSorted && (workingArrayLinearReadPos < workingArrayLinearSize)) {
        workingArrayLinearReadPos++;

        //DEBUG
        printf("\nReturning key %i with value %i", workingArrayLinear[workingArrayLinearReadPos - 1].key,
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
void hlsLinearSort(comparePair *workingArray, int16_t inputSize) {
#pragma HLS INTERFACE m_axi port=workingArray bundle=gmem
#pragma HLS INTERFACE s_axilite port=workingArray bundle=control
#pragma HLS INTERFACE s_axilite port=inputSize bundle=control
#pragma HLS INTERFACE s_axilite port=return bundle=control

    // Localized arrays
    comparePair localInputArray[ListSize];
    comparePair localFinalArray[ListSize + 1];
#pragma HLS ARRAY_PARTITION variable=localFinalArray complete dim=1


        // Copy into local array
        for (int16_t workingArrayPosA = 0; workingArrayPosA < inputSize; workingArrayPosA++) {
#pragma HLS PIPELINE
            localInputArray[workingArrayPosA] = workingArray[workingArrayPosA];
        }


        // Dummy value
        localFinalArray[0].key = INT16_MIN;
        localFinalArray[0].data = INT64_MIN;
        // Handle first values
        localFinalArray[1] = localInputArray[0];

        for (int16_t inputNumber = 1; inputNumber < inputSize; inputNumber++) {
            // Loop through the input array and read each value to insert into the sorted list

            // Put our working value into a register
            comparePair inputPair = localInputArray[inputNumber];


            for (int16_t finalListSlot = ListSize; finalListSlot > 0; finalListSlot--) {
#pragma HLS UNROLL
                // Parallel read of all values in the finalized list

                // Put all working values into registers
                comparePair finalSlotValueAbove;
                finalSlotValueAbove = localFinalArray[finalListSlot - 1];
                comparePair finalSlotValue = localFinalArray[finalListSlot];
                comparePair finalSlotValueNew = finalSlotValue;

                if (hlsArrayCellIsEmpty(finalListSlot, inputNumber) &&
                    (!hlsArrayCellIsEmpty((finalListSlot - 1), inputNumber))) {
                    // This cell is empty, above is not
                    if (inputPair.data > finalSlotValueAbove.data) {
                        // Input value is larger than above value. This goes here
                        finalSlotValueNew = inputPair;
                    } else {
                        // input value is smaller than above value. This goes earlier in the list and we receive cascade
                        finalSlotValueNew = finalSlotValueAbove;
                    }

                } else if (!hlsArrayCellIsEmpty(finalListSlot, inputNumber)) {
                    // This cell is filled
                    if (inputPair.data < finalSlotValueAbove.data) {
                        // Input value is smaller than above value. This goes earlier in the list and we receive cascade
                        finalSlotValueNew = finalSlotValueAbove;
                    } else if (inputPair.data < finalSlotValue.data) {
                        // input value is larger than above value but smaller than ours. This goes here
                        finalSlotValueNew = inputPair;
                    }
                }

                // Update this slow with new value
                localFinalArray[finalListSlot] = finalSlotValueNew;
            }

        }

        // Copy back into passed in array
        for (int16_t workingArrayPosB = 0; workingArrayPosB < inputSize; workingArrayPosB++) {
#pragma HLS PIPELINE
            workingArray[workingArrayPosB] = localFinalArray[workingArrayPosB + 1];
        }

    //}
}

/*
 *  Quick checking function for empty slot
 */
bool hlsArrayCellIsEmpty(int16_t curSlot, int16_t inputPos) {
    if (curSlot < 0) {
        return false;
    }
    return (curSlot >= inputPos + 1);
}
