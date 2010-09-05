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

struct sMacierz33
{
	double m00;
	double m01;
	double m02;
	double m10;
	double m11;
	double m12;
	double m20;
	double m21;
	double m22;
};

struct sSortZ
{
	double z;
	int i;
};

//int abs(int v);
int Random(int max);
double CosInterpolation(double x1, double x2, int res, int n);
void MacierzXVector(sVector &v1, sVector &v2, sMacierz33 &m);
double dMax(double a, double b);
double dMin(double a, double b);
void QuickSortZBuffer(sSortZ *dane, int l, int p);
#endif /* COMMON_MATH_H_ */
