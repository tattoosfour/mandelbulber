/*
 * primitives.hpp
 *
 *  Created on: 15-08-2011
 *      Author: krzysztof
 */

#ifndef PRIMITIVES_H_
#define PRIMITIVES_H_

#include "algebra.hpp"

double PrimitivePlane(CVector3 point, CVector3 centre, CVector3 normal);
double PrimitiveInvertedBox(CVector3 point, CVector3 center, CVector3 size);
double PrimitiveBox(CVector3 point, CVector3 center, CVector3 size);
double PrimitiveSphere(CVector3 point, CVector3 center, double radius);
double PrimitiveInvertedSphere(CVector3 point, CVector3 center, double radius);
double PrimitiveWater(CVector3 point, double height, double amplitude, double length, int iterations);

#endif /* PRIMITIVES_HPP_ */
