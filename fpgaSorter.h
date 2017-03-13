//
// Created by Krittercon on 3/12/2017.
//

#ifndef SQLPROJECT_FPGASORTER_H
#define SQLPROJECT_FPGASORTER_H

#include <stdint.h>
#include <stdbool.h>

#define ListSize 10

#define PaddingSize sizeof(int8_t)
#define KeySize sizeof(int16_t)
#define DataSize sizeof(int64_t)
#define ComparePairSize (PaddingSize + KeySize + DataSize)

typedef struct{
    int8_t paddingData;
    int16_t key;
    int64_t data;
} comparePair;

//----------------------------------------------------------------
//  Forward declarations
//----------------------------------------------------------------
void fpgaSorterInitialize();
void fpgaSorterInsert(int key, char * data, int size);
void fpgaSorterSortLinear();
int fpgaSorterGetLinearResultNext();
void hlsLinearSort(comparePair * workingArray, int16_t inputSize);
bool hlsArrayCellIsEmpty(int16_t curSlot, int16_t inputPos);

#endif //SQLPROJECT_FPGASORTER_H
