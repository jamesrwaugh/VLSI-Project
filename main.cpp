#include <iostream>
#include "stdcell.h"
#include "module.h"
#include "utility.h"
#include "kerninghan.h"
#include "output.h"

int main(int argc, char** argv)
{
    //If not enough arguments print usage
    //if(argc < 3) {
    //    std::cout << "Usage: " << argv[0] << " <stdcell file> <module file>" << std::endl;
     //   return 1;
    //}

    //try
    //{
        //Loads all files and information
        MattCellFile cells("usf_ami05_std_cells.lib");
        std::vector<module> modules = readModuleFile("C7552.netblif", cells);
        SubcktFile ckts("slice00.subckts", 0, cells);
        
        //Partitions each module and writes .subckts
        for(const module& m : modules)
        {
            auto partitions = kernighanLin(m);

            std::cout << "Writing partitions for \"" << partitions.first.name << "\"" << std::endl;
            ckts << partitions.first  << std::endl;
            ckts << partitions.second << std::endl;
        }
   // }
    //catch(std::exception& e) {
    //    std::cerr << e.what() << std::endl;
    //}
    
    return 0;
}
