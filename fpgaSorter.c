//
// Created by Krittercon on 3/12/2017.
// Bitonic version
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

void merge_up(int *arr, int n) {
    int step=n/2,i,j,k,temp;
    while (step > 0) {
        for (i=0; i < n; i+=step*2) {
            for (j=i,k=0;k < step;j++,k++) {
                if (arr[j] > arr[j+step]) {
                    // swap
                    temp = arr[j];
                    arr[j]=arr[j+step];
                    arr[j+step]=temp;
                }
            }
        }
        step /= 2;
    }
}

void merge_down(int *arr, int n) {
    int step=n/2,i,j,k,temp;
    while (step > 0) {
        for (i=0; i < n; i+=step*2) {
            for (j=i,k=0;k < step;j++,k++) {
                if (arr[j] < arr[j+step]) {
                    // swap
                    temp = arr[j];
                    arr[j]=arr[j+step];
                    arr[j+step]=temp;
                }
            }
        }
        step /= 2;
    }
}

// do merges
void do_sort(int * arr, int n) {
    for (int s=2; s <= n; s*=2) {
        for (int i=0; i < n;) {
            merge_up((arr+i),s);
            merge_down((arr+i+s),s);
            i += s*2;
        }
    }
}

void dummyFPGATrap(){
    int i = 1;
    int b = 1;
    int c = i + b;
}

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
    printf("\nAdding key %d with value %d", key, workingArrayLinear[workingArrayLinearSize].data);

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
        printf("\nReturning key %d with value %d", workingArrayLinear[workingArrayLinearReadPos - 1].key, workingArrayLinear[workingArrayLinearReadPos - 1].data);

        return workingArrayLinear[workingArrayLinearReadPos - 1].key;
    }

    return 0;
}

void hlsLinearSort(comparePair * workingArray, int inputSize){
//#pragma HLS INTERFACE m_axi port=workingArray bundle=gmem
//#pragma HLS INTERFACE s_axilite port=workingArray bundle=control
//#pragma HLS INTERFACE s_axilite port=inputSize bundle=control
//#pragma HLS INTERFACE s_axilite port=return bundle=control

    // Localized arrays
    comparePair localInputArray[ListSize];
    comparePair localFinalArray[ListSize];
//#pragma HLS ARRAY_PARTITION variable=localFinalArray complete

    // Temp
    int tempArray[inputSize];

    // Copy into local array
    //memcpy(localInputArray, workingArray, (inputSize * ComparePairSize));

    for(int i = 0; i < inputSize; i++){
        printf("\nCopying in value %d", workingArray[i].data);
        tempArray[i] = workingArray[i].data;
    }

    for (int s=2; s <= inputSize; s*=2) {
        for (int i=0; i < inputSize;) {
            merge_up((tempArray+i),s);
            merge_down((tempArray+i+s),s);
            i += s*2;
        }
    }

    for(int j = 0; j < inputSize; j++){
        workingArray[j].data = tempArray[j];
        printf("\nCopying out value %d", tempArray[j]);
    }

    // Copy back into passed in array
   // memcpy(workingArray, localFinalArray, (inputSize * ComparePairSize));
}
