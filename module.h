#ifndef MODULE_H
#define MODULE_H

#include <string>
#include <vector>
#include "stdcell.h"

struct module
{
    //Ordered list of gates as definitions only
    std::vector<stdcell> gates;
    
    //Connectivity matrix of the module gates
    std::vector<std::vector<int>> connections;
    
    //Module name
    std::string name;
};


/* roger
 * Loads and parses a .netblif file and returns a vector of all modules in the file,
 * with their connectivity matricies and standard cell gates.
 * Uses a MattCellFile to check and load standard cell information
 */
std::vector<module> readModuleFile(const std::string& fileName, MattCellFile& cells);

#endif
