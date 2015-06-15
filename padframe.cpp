#include <sstream>
#include <fstream>
#include "utility.h"
#include "padframe.h"

PadframeFile::PadframeFile(const std::string& filename)
{
    usable_width = usable_height = slices_horiz = slices_verical = 0;

    //Error if unable to open file
    std::ifstream file(filename);
    if(!file.is_open()) {
        error("Padframe definition file \"", filename, "\" not found");
    }

    //Read in file data scanning what we want
    bool usableFound = false, slicesFound = false;
    std::string line;
    std::istringstream ss;
    while(std::getline(file, line))
    {
        if(line.find(".USABLE") != std::string::npos) {
            ss.str(line);
            ss.ignore(7) >> usable_width >> usable_height;
            usableFound = true;
        }
        else if(line.find(".SLICES") != std::string::npos) {
            ss.str(line);
            ss.ignore(7) >> slices_horiz >> slices_verical;
            slicesFound = true;
        }
        else if(usableFound && slicesFound) {
            break;
        }
    }

    //Error if we did not find .SLICES or .USABLE lines
    if(!(usableFound && slicesFound)) {
        error("Padframe file \"", filename, "\" missing .USABLE or .SLICES");
    }
}

int PadframeFile::usableWidth() const
{
    return usable_width;
}
 
int PadframeFile::usableHeight() const
{
    return usable_height;
}
 
int PadframeFile::slicesHoriz() const
{
    return slices_horiz;
}
 
int PadframeFile::slicesVert() const
{
    return slices_verical;
}

