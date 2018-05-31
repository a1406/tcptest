#ifndef UNIT_PATH_H
#define UNIT_PATH_H

#include <stdint.h>
//#include "comm_message.pb-c.h"
//class unit_struct;

struct position
{
	float pos_x;
	float pos_z;
};
#define MAX_PATH_POSITION (256)
struct unit_path
{
	uint64_t start_time;
	uint8_t cur_pos;
	uint8_t max_pos;
	struct position pos[MAX_PATH_POSITION];
	float direct_x;  //按方向移动
	float direct_z;
};
int get_distance_square(struct position *start, struct position *end);
float getdistance(struct position *start, struct position *end);
float FastSqrtQ3(float x);

double pos_to_angle(double x, double z);
void rotata_point(double angle, double x, double z);

typedef double qreal;
qreal qFastSin(qreal x);
qreal qFastCos(qreal x);

typedef bool (*rasterize_path_func)(int x, int y);
void rasterize_path_point(int xs, int ys, int xe, int ye, rasterize_path_func func);

double c_angle_to_unity_angle(double angle);
double unity_angle_to_c_angle(double angle);
#endif /* UNIT_PATH_H */
