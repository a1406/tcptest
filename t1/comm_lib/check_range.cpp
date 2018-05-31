#include "check_range.h"
#include <stdlib.h>
#include <math.h>

bool check_distance_in_range(const struct position *pos1, const struct position *pos2, const double range)
{
	if (fabsf(pos1->pos_x - pos2->pos_x) > range)
		return false;
	if (fabsf(pos1->pos_z - pos2->pos_z) > range)
		return false;
	return true;
}

bool check_circle_in_range(const struct position *pos1, const struct position *pos2, const double range)
{
	double x = (pos1->pos_x - pos2->pos_x);
	double z = (pos1->pos_z - pos2->pos_z);

	if (x * x + z * z > range * range)
		return false;
	return true;
}
