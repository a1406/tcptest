#include "GRand.h"
#include <math.h>
#include <assert.h>

void GRand::setRandSeed(double m)
{
//    assert(m != 0);
	g_seed = m;
	g_last = g_seed;
}

//线性同余法
double GRand::getRand()
{
	const static double a = 32719;
	const static double m = pow(2.0, 31.0);
	double t = fmod((a*g_last),(m-1));
	if(t<=m/2)     //与人字映射结合生成随机数
	{
		t=2*t;
	}
	else
	{
		t=2*(m-t)+1;
	}

	g_last = t;
	return t/(m-1);
}

//平方取中法
double GRand::getRand2()
{
	const static double pow1 = pow(10.0,5.0);
	const static double pow2 = pow(10.0,10.0);
	double t = g_last * g_last;
	t = t / pow1;
	t = fmod(t,pow2);
	g_last = t;
	return t/pow2;
}
