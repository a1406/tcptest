#ifndef CHECK_RANGE_H
#define CHECK_RANGE_H

#include "unit_path.h"

bool check_distance_in_range(const struct position *pos1, const struct position *pos2, const double range);
bool check_circle_in_range(const struct position *pos1, const struct position *pos2, const double range);

#endif /* CHECK_RANGE_H */
