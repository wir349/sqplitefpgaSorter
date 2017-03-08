/*
*	CS259 Project - Linear Shift + Merge Sort for FPGAs
*	UCLA WI2017
*	Krit Sae Fang
*/
#ifndef FPGASORTER_FPGASORTER_H
#define FPGASORTER_FPGASORTER_H

#endif //FPGASORTER_FPGASORTER_H

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

	//---------------------------------------------------------------------------------------------
	// "Public" functions and methods
	//---------------------------------------------------------------------------------------------
	
	/*
	*	Initializer
	*	Prepares values for working with this sorter
	*/
	void fpgaSorterInitialize();

	/*
	*	Adds value to be linear sorted
	*	Value is kept off BRAM until ready.
	*/
	void fpgaSorterInsert(
		char *data,
		int size
	);
	/*void fpgaSorterInsert(
		int key,
		char *data,
		int size
	);*/

	/*
	*	Quick reset
	*	Returns counter to zero
	*/
	void fpgaSorterResetLinear();
	
	/*
	*	Sorts the values in our working array
	*	Does not yet return anything
	*/
	void fpgaSorterSortLinear();
	
	/*
	*	Gets the sorted array
	*	Copies the result array into the provided array
	*	Not completely safe. Array needs to be large enough to accomadate
	*	Uses the struct defined in this file
	*	If not yet sorted, this does nothing.
	*/
	void fpgaSorterGetLinearResultArray(char *returnArray);
	
	/*
	*	Gets a key from the sorted array. Returns 0 if not sorted
	*/
	int fpgaSorterGetLinearResultPos(int pos);
	
	/*
	*	Gets a key from the sorted array. Returns 0 if not sorted or reached end
	*	Returns values in sorted other each time it is called
	*/
	int fpgaSorterGetLinearResultNext();
	
	/*
	*	Resets the read pointer to 0.
	*/
	void fpgaSorterResetLinearRead();

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
	);

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
	static bool dataCompare(comparePair x, comparePair y);

	/*
	*	Helper function for checking for empty cell.
	*	Makes life easier since I don't have to do multiple nests after nests
	*/
	static bool arrayCellIsEmpty(i,j);

} // End fpga sorter class
