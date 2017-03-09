/*
*	CS259 Project - Linear Shift + Merge Sort for FPGAs
*	UCLA WI2017
*	Krit Sae Fang
*/
#include "fpgaSorter.h"
#include <stdbool.h>
#include <string.h>

// Constants
#define ListSize 1000					// Assumed list size (Depends on available BRAM blocks)
#define DataSize 10					// 80 bits for comparision data (10 characters)
#define StructSize 12

// Constants
#define InputBufferSize 400			// Used to save memory
#define InputBufferMin 200				// Minimum in buffer before a refill

	
	//---------------------------------------------------------------------------------------------
	// Declarations and stuff
	//---------------------------------------------------------------------------------------------

	// Saved array for working with (kept on disk/DRAM)
	char blankArray[DataSize];
	
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
	void fpgaSorterInitialize()
	{
		// Set up blank array.
		for(int i=0; i < DataSize ; i++){
			blankArray[i] = '\0';
		}
		
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
	)
	{	
		if(workingArrayLinearSize > ListSize){
			// Exceeded max defined size. Return without adding.
			return;
		}

		// Copy in the key/pointer
		workingArrayLinear[workingArrayLinearSize].key = key;
		
		// Copy in data to compare
		if(size > 10){
			memcpy(workingArrayLinear[workingArrayLinearSize].data, data, DataSize);
		} else {
			memcpy(workingArrayLinear[workingArrayLinearSize].data, data, size);
			// Pad out remaining values (if any)
			memcpy(workingArrayLinear[workingArrayLinearSize].data + size, blankArray, DataSize - size);
		}
		
		// Add value to local counter
		workingArrayLinearSize++;
	}

	/*
	*	Quick reset
	*	Returns counter to zero
	*/
	void fpgaSorterResetLinear(){
		workingArrayLinearSize = 0;
		isSorted = false;
	}
	
	/*
	*	Sorts the values in our working array
	*	Does not yet return anything
	*/
	void fpgaSorterSortLinear(){
		if(!isSorted){
			hlsLinearSort(workingArrayLinear, workingArrayLinearSize);
			isSorted = false;
		}
	}
	
	/*
	*	Gets the sorted array
	*	Copies the result array into the provided array
	*	Not completely safe. Array needs to be large enough to accomadate
	*	Uses the struct defined in this file
	*	If not yet sorted, this does nothing.
	*/
	void fpgaSorterGetLinearResultArray(char *returnArray){
		if(isSorted){
			memcpy(returnArray, workingArrayLinear, workingArrayLinearSize*StructSize);
		}
	}
	
	/*
	*	Gets a key from the sorted array. Returns 0 if not sorted
	*/
	int fpgaSorterGetLinearResultPos(int pos){
		if(isSorted){
			return workingArrayLinear[pos].key;
		}
		return 0;
	}
	
	/*
	*	Gets a key from the sorted array. Returns 0 if not sorted or reached end
	*	Returns values in sorted other each time it is called
	*/
	int fpgaSorterGetLinearResultNext(){
		if(isSorted && (workingArrayLinearReadPos < workingArrayLinearSize)){
			workingArrayLinearReadPos++;
			return workingArrayLinear[workingArrayLinearReadPos - 1].key;
		}
		return 0;
	}
	
	/*
	*	Resets the read pointer to 0.
	*/
	void fpgaSorterResetLinearRead(){
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
	)
	{
		//#pragma HLS
		
		// Localized input array. 
		comparePair pLocal[ListSize];
		// comparePair pLocal[InputBufferSize];
		
		// Localized result arrays
		comparePair pFinal[ListSize];
		comparePair pFinalBuffer[ListSize];
		//#pragma HLS ARRAY_PARTITION
		
		// Copy into local BRAM.
		memcpy(pLocal, p, ListSize*StructSize);
		// cmemcpy(pLocal, p, InputBufferSize*StructSize);
		
		// First value handling. Makes life easier with the loop
		pFinal[0] = pLocal[0];
		
		for(int i = 1; i < pSize; i++){
		// This loop cannot be unrolled or pipelined without collision or other issues
			for(int j = 1; j < ListSize; j++){
				//#pragma HLS UNROLL
				
				// If the item in this cell is bigger and not empty...
				if(dataCompare(pFinal[j], pLocal[i]) && !arrayCellIsEmpty(j,i) ){
					// Cascade move down to open up the slot we need
					pFinalBuffer[j+1] = pFinal[j];
					
					// If the item above us is smaller than the incoming value
					// Copy our value in
					if(dataCompare(pLocal[i], pFinal[j-1])){
						pFinalBuffer[j] = pLocal[i];
					}
				}
				
				// If we have an empty cell and the item above is smaller...
				if(arrayCellIsEmpty(j,i) && dataCompare(pLocal[i], pFinal[j-1])){
					pFinalBuffer[j] = pLocal[i];
				}
			}
			// Copy values from our safety buffer back into the working array.
			memcpy(pFinalBuffer, p, ListSize*StructSize);
		}
		
		// Copy back into DRAM for transfer back into CPU processing
		memcpy(p, pLocal, ListSize*StructSize);
	}

	/*
	*	Helper function for vopying arrays.
	*/
	//static copyArray(comparePair source, comparePair destination, int size){
	//	memcpy(source, destination, size);
	//}

	/*
	*	Helper function for comparing values in the struct
	*	Bitwise, byte by byte via char
	*/
	bool dataCompare(comparePair x, comparePair y){
		for(int i = 0; i < DataSize; i++){
			if(x.data[i] > y.data[i]){
				return true;
			}
		}
		return false;
	}

	/*
	*	Helper function for checking for empty cell.
	*	Makes life easier since I don't have to do multiple nests after nests
	*/
	bool arrayCellIsEmpty(int i,int j){
		if(i >= j){
			return true;
		}
		return false;
	}

	/*
	*	Merge sort implementation for FPGAs
	*	INCOMPLETE CODE
	*/
	static void hlsMergeSort(
		char *p1,
		int p1Size,
		char *p2,
		int p2Size,
		char *pFinal
	)
	{
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
		while(p1Remain > 0 || p2Remain > 0){
			
			// Move local final list to DRAM if filled
			if(currentFinalLocalSize > ListSize - 1){
				memcpy(pFinal + (currentFinalSize*StructSize), pFinalLocal, ListSize*StructSize);
				currentFinalSize = currentFinalSize + currentFinalLocalSize;
			}
			
			// Fill local p1 if empty
			if(p1Remain > 0 && p1LocalSize == 0){
				if(p1Remain < InputBufferSize){
					memcpy(p1Local, p1 + (p1Moves * DataSize * ListSize), InputBufferSize*StructSize);
					p1Remain = p1Remain - InputBufferSize;
					p1LocalSize = InputBufferSize;
				} else {
					memcpy(p1Local, p1 + (p1Moves * DataSize * ListSize), p1Remain*StructSize);
					p1Remain = 0;
					p1LocalSize = p1Remain;
				}
				p1Moves++;
			}
			
			// Fill local p2 if empty
			if(p2Remain > 0 && p2LocalSize == 0){
				if(p2Remain < InputBufferSize){
					memcpy(p1Local, p2 + (p2Moves * DataSize * ListSize), InputBufferSize*StructSize);
					p2Remain = p2Remain - InputBufferSize;
					p2LocalSize = InputBufferSize;
				} else {
					memcpy(p1Local, p2 + (p2Moves * DataSize * ListSize), p1Remain*StructSize);
					p2Remain = 0;
					p2LocalSize = p2Remain;
				}
				p2Moves++;
			}
		}
		
		// Copy into local BRAM.
		memcpy(p1Local, p1, p1Size*StructSize);
		memcpy(p2Local, p2, p2Size*StructSize);
		
		
	}
