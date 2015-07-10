#ifndef FLOORPLAN_H
#define FLOORPLAN_H
#include "module.h"

/* VLSI Floorplanning Implementation
 * Input: A module to perform floorplanning on
 * Output: A polish expression representing an efficient slicing
 *  floorplan placement */

std::vector<std::string> floorplan(const module& partition);

#endif
