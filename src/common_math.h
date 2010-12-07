/*
 * common_math.h
 *
 *  Created on: 2010-01-23
 *      Author: krzysztof marczak
 */

#ifndef COMMON_MATH_H_
#define COMMON_MATH_H_

#include "algebra.hpp"

struct sVectorsAround
{
	double alfa;
	double beta;
	CVector3 v;
	int R;
	int G;
	int B;
	bool notBlack;
};

struct sVector
{
	double x;
	double y;
	double z;
};

struct sSortZ
{
	double z;
	int i;
};

//int abs(int v);
int Random(int max);
double dMax(double a, double b, double c);
void QuickSortZBuffer(sSortZ *dane, int l, int p);
#endif /* COMMON_MATH_H_ */
