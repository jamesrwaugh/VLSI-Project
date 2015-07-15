#ifndef KERNIGHAN_LIN_H
#define KERNIGHAN_LIN_H
#include <vector>
#include "module.h"
#include "padframe.h"

/* Implementation of the Kernighan–Lin two-way graph partitioning algorithm.
 * Input: A module to be partitioned
 * Output: Two modules, partition A and partition B of the module */

std::pair<module,module> kernighanLin(const module& m);

/* Kerninghan-Lin two-way partitioning algorithm, but continues to recursively partition a
 * moudule in two until mostly all areas are less than the area of a usable padfram slice
 * Input: A module to partition and a padframe to judge width/lengths
 * Output: K partitions, each corresponding to <= a slice width/height
 */

std::vector<module> kerninghanLinPadframeSlice(const module& m, const PadframeFile& f);

#endif
