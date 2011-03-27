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

enum enumPerspectiveType
{
	threePoint = 0, fishEye = 1, equirectangular = 2
};

//int abs(int v);
int Random(int max);
double dMax(double a, double b, double c);
void QuickSortZBuffer(sSortZ *dane, int l, int p);
CVector3 Projection3D(CVector3 point, CVector3 vp, CRotationMatrix mRot, enumPerspectiveType perspectiveType, double fov, double zoom);

#endif /* COMMON_MATH_H_ */
