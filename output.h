#ifndef OUTPUT_H
#define OUTPUT_H
#include <string>
#include <fstream>
#include "module.h"
#include "stdcell.h"
#include "floorplan.h"

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


/* UnityFile is an iterface to a format needed by Unity to perform floorplanning */

class UnityFile
{
public:
    UnityFile(const std::string& filename);

    //Writes all partitioned modules along with their polishes
    void write(std::vector<module>& modules, std::vector<polish_string>& polishes);

private:
    std::ofstream file;
};

#endif
