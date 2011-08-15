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
double PrimitiveInverseBox(CVector3 point, CVector3 corner1, CVector3 corner2);
double PrimitiveBox(CVector3 point, CVector3 corner1, CVector3 corner2);
double PrimitiveSphere(CVector3 point, CVector3 center, double radius);
double PrimitiveInverseSphere(CVector3 point, CVector3 center, double radius);

#endif /* PRIMITIVES_HPP_ */
