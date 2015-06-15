#ifndef KERNIGHAN_LIN_H
#define KERNIGHAN_LIN_H
#include <vector>
#include "module.h"

/* Implementation of the Kernighan–Lin two-way graph partitioning algorithm.
 * Input: A module to be partitioned
 * Output: Two modules, partition A and partition B of the module */

std::pair<module, module> kernighanLin(const module& m);

#endif
