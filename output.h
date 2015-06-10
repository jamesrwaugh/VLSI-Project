#ifndef OUTPUT_H
#define OUTPUT_H
#include <string>
#include "module.h"
#include "stdcell.h"

/* Given a module, returns the .subckt text as a string to be written in
 * the slice .subckts file. `sliceNum` is the number of the slice containing the 
 * partition, and `partitionNum` is the number of the partition in the slice */
std::string getSubcktText(const module& partition, const MattCellFile& cells, int sliceNum, int partitionNum);

#endif
