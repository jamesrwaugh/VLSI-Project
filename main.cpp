#include <iostream>
#include "stdcell.h"
#include "module.h"
#include "utility.h"

int main(int argc, char** argv)
{
    MattCellFile cells("usf_ami05_std_cells.lib");
    std::cout << cells << std::endl;
    
    std::vector<module> models = readModuleFile(argv[1], cells);

    //Outputs connectivity matricies
    for(module& m : models) {
        for(std::vector<int>& row : m.connections) {
            std::cout << row << std::endl;
        }
        std::cout << std::endl;
    }
    
    return 0;
}
