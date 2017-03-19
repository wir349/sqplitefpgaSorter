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
comparePair workingArrayLinear[ListSize];
int workingArrayLinearSize;
int workingArrayLinearReadPos;

/*
 *  Initializer
*/
void fpgaSorterInitialize(){
    if(!initialized){
        isSorted = false;
        workingArrayLinearSize = 0;
        workingArrayLinearReadPos = 0;

        initialized = true;
    }
}

/*
 *  Adds an entry
 */
void fpgaSorterInsert(int key, char * data, int size){
    if(workingArrayLinearSize > ListSize){
        return;
    }

    if(!initialized){
        fpgaSorterInitialize();
    }

    //DEBUG
    key = workingArrayLinearSize;

    workingArrayLinear[workingArrayLinearSize].key = key;
    memcpy(&workingArrayLinear[workingArrayLinearSize].data, data, DataSize);

    //DEBUG
    workingArrayLinear[workingArrayLinearSize].data = rand();
    printf("\nAdding key %i with value %i", key, workingArrayLinear[workingArrayLinearSize].data);

    workingArrayLinearSize++;
}

void fpgaSorterSortLinear() {
    if(!isSorted){
        hlsLinearSort(workingArrayLinear, workingArrayLinearSize);
        isSorted = true;
    }
}

int fpgaSorterGetLinearResultNext(){
    if(isSorted && (workingArrayLinearReadPos < workingArrayLinearSize)) {
        workingArrayLinearReadPos++;

        //DEBUG
        printf("\nReturning key %i with value %i", workingArrayLinear[workingArrayLinearReadPos - 1].key, workingArrayLinear[workingArrayLinearReadPos - 1].data);

        return workingArrayLinear[workingArrayLinearReadPos - 1].key;
    }

    return 0;
}

void hlsLinearSort(comparePair * workingArray, int16_t inputSize){
#pragma HLS INTERFACE m_axi port=workingArray bundle=gmem
#pragma HLS INTERFACE s_axilite port=workingArray bundle=control
#pragma HLS INTERFACE s_axilite port=inputSize bundle=control
#pragma HLS INTERFACE s_axilite port=return bundle=control

    // Localized arrays
    comparePair localInputArray[ListSize];
    comparePair localFinalArray[ListSize];
#pragma HLS ARRAY_PARTITION variable=localFinalArray complete dim=1
#pragma HLS RESOURCE variable=localFinalArray core=RAM_2P_BRAM

    // Copy into local array
    memcpy(localInputArray, workingArray, (inputSize * ComparePairSize));

    // Handle first value
    localFinalArray[0] = localInputArray[0];

    for (int16_t inputNumber = 1; inputNumber < inputSize; inputNumber++){
        // Loop through the input array and read each value to insert into the sorted list

        // Put our working value into a register
        comparePair inputPair = localInputArray[inputNumber];

        for(int16_t finalListSlot = (ListSize-1); finalListSlot >= 0; finalListSlot--){
#pragma HLS UNROLL
            // Parallel read of all values in the finalized list

            // Put all working values into registers
            comparePair finalSlotValueAbove;
            if (finalListSlot == 0){
                finalSlotValueAbove.data = 0;
            } else {
                finalSlotValueAbove = localFinalArray[finalListSlot - 1];
            }
            comparePair finalSlotValue = localFinalArray[finalListSlot];
            comparePair finalSlotValueNew = finalSlotValue;

            if(hlsArrayCellIsEmpty(finalListSlot, inputNumber) && (!hlsArrayCellIsEmpty((finalListSlot - 1), inputNumber))){
                // This cell is empty, above is not
                if(inputPair.data > finalSlotValueAbove.data){
                    // Input value is larger than above value. This goes here
                    finalSlotValueNew = inputPair;
                } else {
                    // input value is smaller than above value. This goes earlier in the list and we receive cascade
                    finalSlotValueNew = finalSlotValueAbove;
                }

            } else if(!hlsArrayCellIsEmpty(finalListSlot, inputNumber)) {
                // This cell is filled
                if(inputPair.data < finalSlotValueAbove.data){
                    // Input value is smaller than above value. This goes earlier in the list and we receive cascade
                    finalSlotValueNew = finalSlotValueAbove;
                } else if(inputPair.data < finalSlotValue.data) {
                    // input value is larger than above value but smaller than ours. This goes here
                    finalSlotValueNew = inputPair;
                }
            }

            // Update this slow with new value
            localFinalArray[finalListSlot] = finalSlotValueNew;
        }

    }

    // Copy back into passed in array
    memcpy(workingArray, localFinalArray, (inputSize * ComparePairSize));
}

/*
 *  Quick checking function for empty slot
 */
bool hlsArrayCellIsEmpty(int16_t curSlot, int16_t inputPos){
    if(curSlot < 0){
        return false;
    }
    return (curSlot >= inputPos);
}
