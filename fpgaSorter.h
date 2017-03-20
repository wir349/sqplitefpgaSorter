//
// Created by Krittercon on 3/12/2017.
//

#ifndef SQLPROJECT_FPGASORTER_H
#define SQLPROJECT_FPGASORTER_H

#include <stdint.h>
#include <stdbool.h>

#define ListSize 400

#define KeySize sizeof(int)
#define DataSize sizeof(int)
#define ComparePairSize (KeySize + DataSize)

typedef struct{
    int key;
    int data;
} comparePair;

//----------------------------------------------------------------
//  Forward declarations
//----------------------------------------------------------------
void merge_up(int *arr, int n);
void merge_down(int *arr, int n);
void do_sort(int * arr, int n);
void fpgaSorterInitialize();
void fpgaSorterInsert(int key, char * data, int size);
void fpgaSorterSortLinear();
int fpgaSorterGetLinearResultNext();
void hlsLinearSort(comparePair * workingArray, int inputSize);

#endif //SQLPROJECT_FPGASORTER_H
