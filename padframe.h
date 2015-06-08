#ifndef PADFRAME_H
#define PADFRAME_H

#include <string>

class PadframeFile
{
public:
    PadframeFile(const std::string& filename);
    int usableWidth() const;
    int usableHeight() const;
    int slicesHoriz() const;
    int slicesVert() const;

private:
    int usable_width;
    int usable_height;
    int slices_horiz;
    int slices_verical;
};

#endif

