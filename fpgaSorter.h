#ifndef SQLPROJECT_FPGASORTER_H
#define SQLPROJECT_FPGASORTER_H

#include <stdint.h>
#include <stdbool.h>

#define ListSize 400

#define KeySize sizeof(int16_t)
#define DataSize sizeof(int64_t)
#define ComparePairSize (KeySize + DataSize)

typedef struct{
    int16_t key;
    int64_t data;
} comparePair;

//----------------------------------------------------------------
//  Forward declarations
//----------------------------------------------------------------
void merge_up(comparePair *arr, int16_t n);
void merge_down(comparePair *arr, int16_t n);
void fpgaSorterInitialize();
void fpgaSorterInsert(int key, char * data, int size);
void fpgaSorterSortLinear();
int fpgaSorterGetLinearResultNext();
void hlsLinearSort(comparePair * workingArray, int16_t inputSize);

#endif //SQLPROJECT_FPGASORTER_H
