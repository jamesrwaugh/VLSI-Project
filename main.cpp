#include <iostream>
#include "stdcell.h"
#include "module.h"
#include "utility.h"

int main(int argc, char** argv)
{
    //If not enough arguments print usage
    if(argc < 3) {
        std::cout << "Usage: " << argv[0] << " <stdcell file> <module file>" << std::endl;
        return 1;
    }

    try 
    {
        //Loads all files
        MattCellFile cells(argv[1]);
        std::vector<module> modules = readModuleFile(argv[2], cells);    
        
        //Outputs connectivity matricies
        for(module& m : modules) {
            for(std::vector<int>& row : m.connections)
                std::cout << row << std::endl;
            std::cout << std::endl;
        }        
    } 
    catch(std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    
    return 0;
}
