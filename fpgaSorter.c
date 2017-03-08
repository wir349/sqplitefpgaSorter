/*
*	CS259 Project - Linear Shift + Merge Sort for FPGAs
*	UCLA WI2017
*	Krit Sae Fang
*/
#include <fpgaSorter.h>

// Constants
#define ListSize 1000;					// Assumed list size (Depends on available BRAM blocks)
#define DataSize 10;					// 80 bits for comparision data (10 characters)
#define StructSize 12;

// Constants
#define InputBufferSize 400;			// Used to save memory
#define InputBufferMin 200;				// Minimum in buffer before a refill

class fpgaSorter {
	
	//---------------------------------------------------------------------------------------------
	// Declarations and stuff
	//---------------------------------------------------------------------------------------------

	// Struct declarations
	typedef struct comparePair comparePair;	// Structured pair of ID and comparision data

	// Struct for our ptr/compare value pair.
	struct comparePair{
		int key;				//ptr to actual object
		char data[DataSize];	//Data to compare
	}

	// Saved array for working with (kept on disk/DRAM)
	static char blankArray[DataSize];
	
	// General function counters and keepers
	static bool isSorted;

	static comparePair workingArrayLinear[ListSize];
	static int workingArrayLinearSize;
	
	static int workingArrayLinearReadPos;

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
			blankArray[i] = NULL;
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
	/*void fpgaSorterInsert(
		int key,
		char *data,
		int size
	*/)
	void fpgaSorterInsert(
		char *data,
		int size
	)
	{	
		if(workingArrayLinearSize > ListSize){
			// Exceeded max defined size. Return without adding.
			return;
		}

		// Copy in the key/pointer
		// workingArrayLinear[workingArrayLinearSize].key = key;
		
		// Copy in data to compare
		if((size - 1) > 10){
			memcpy[workingArrayLinear[workingArrayLinearSize].data, data + 1, DataSize];
		} else {
			memcpy[workingArrayLinear[workingArrayLinearSize].data, data + 1, (size - 1)];
			// Pad out remaining values (if any)
			memcpy[workingArrayLinear[workingArrayLinearSize].data + (size - 1), blankArray, DataSize - (size - 1)];
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
			memcpy[returnArray, workingArrayLinear, workingArrayLinearSize*StructSize];
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
	static void hlsLinearSort(
		char *p,
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
				if(arrayCellIsEmpty(j,i) && dataCompare(pLocal[i], pFinal[j-1]){
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
	static bool dataCompare(comparePair x, comparePair y){
		for(int i = 0; i < DataSize; i++){
			if(x.data[i] > y.data[i]){
				return true;
			}
		}
		return false
	}

	/*
	*	Helper function for checking for empty cell.
	*	Makes life easier since I don't have to do multiple nests after nests
	*/
	static bool arrayCellIsEmpty(i,j){
		if(i >= j){
			return true;
		}
		return false;
	}

} // End fpga sorter class
