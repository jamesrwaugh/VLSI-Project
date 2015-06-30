#ifndef OUTPUT_H
#define OUTPUT_H
#include <string>
#include <fstream>
#include "module.h"
#include "stdcell.h"

/* SubcktFile is an interface to a .subckt required output file. It has an
 * ostream << operator to write a module as a subckt in the file it is
 * constructed with. Sequentually written modules are given incresing numbers
 * automatically in the file. */

class SubcktFile
{
public:
     SubcktFile(const std::string &filename, int sliceNum, const MattCellFile &cells);

     //Operator to write .subckt text to the file for a partition pair
     std::ostream& operator<<(const std::pair<module,module>& partitions);

private:
    //The header comments of partitions, external wire and gate amounts
    std::string getHeaderText(const std::pair<module, module>& p);

    int sliceNumber;
    int partitionNumber;
    std::ofstream file;
    const MattCellFile& cellsRef;
};

#endif
