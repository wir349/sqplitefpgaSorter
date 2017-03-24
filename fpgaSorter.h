//
// Created by Krittercon on 3/12/2017.
//

#ifndef SQLPROJECT_FPGASORTER_H
#define SQLPROJECT_FPGASORTER_H

#include <stdint.h>
#include <stdbool.h>

#define ListSize 16
#define LogListSize 4

#define KeySize sizeof(int16_t)
#define DataSize sizeof(int64_t)
//#define ComparePairSize (KeySize + DataSize)

typedef struct{
    int16_t key;
    int64_t data;
} ComparePair;

//----------------------------------------------------------------
//  Forward declarations
//----------------------------------------------------------------
void fpgaSorterInitialize(int sortSize);
void fpgaSorterInsert(int key, char * data, int size);
void fpgaSorterSortLinear();
int fpgaSorterGetLinearResultNext();
void hlsLinearSort(ComparePair * workingArray, int16_t inputSize);
bool hlsArrayCellIsEmpty(int16_t curSlot, int16_t inputPos);

#endif //SQLPROJECT_FPGASORTER_H
