#ifndef FLOORPLAN_H
#define FLOORPLAN_H
#include "module.h"

/* VLSI Floorplanning Implementation
 * Input: A module to perform floorplanning on
 * Output: A polish expression representing an efficient slicing
 *  floorplan placement */

typedef std::vector<std::string> polish_string;

//Floorplan a single module
polish_string floorplan(module& partition);

//Pointer version for threading (calls "floorplan")
polish_string floorplan_ptr(module* partitionPtr);

//Uses multithreading to floorplan all modules in `modules`
std::vector<polish_string> floorplan_all(std::vector<module>& modules, unsigned batchSize = 20);

#endif
