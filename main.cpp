#include <iostream>
#include "stdcell.h"
#include "module.h"
#include "utility.h"
#include "kerninghan.h"
#include "output.h"

#if 1
int main(int argc, char** argv)
{
    //If not enough arguments print usage
    if(argc < 3) {
        std::cout << "Usage: " << argv[0] << " <stdcell file> <module file>" << std::endl;
        return 1;
    }

    try 
    {
        //Loads all files and information
        MattCellFile cells(argv[1]);
        std::vector<module> modules = readModuleFile(argv[2], cells);
        SubcktFile ckts("slice00.subckts", 0, cells);
        
        //Partitions each module and writes .subckts
        for(const module& m : modules)
        {
            auto partitions = kernighanLin(m);

            std::cout << "Writing partitions for \"" << partitions.first.name << "\"" << std::endl;
            ckts << partitions;
        }
    } 
    catch(std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    
    return 0;
}
#endif
