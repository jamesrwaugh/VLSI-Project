#ifndef KERNIGHAN_LIN_H
#define KERNIGHAN_LIN_H
#include <vector>
#include "module.h"

/* Implementation of the Kernighan–Lin two-way graph partitioning algorithm.
 * Input: Connectivity matrix from `module` struct.
 * Output: Two vectors of integers corresponding to gates in partition A and B. */
std::pair<std::vector<int>,std::vector<int>> kernighanLin(const std::vector<std::vector<int>>& matrix);

/* Kernighan-Lin; Takes a module and gives back two sub-modules */
std::pair<module, module> kernighanLin(const module& m);

#endif
