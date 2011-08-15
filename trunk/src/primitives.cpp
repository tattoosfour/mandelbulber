/*
 * primitives.cpp
 *
 *  Created on: 15-08-2011
 *      Author: krzysztof marczak
 */

#include "primitives.h"

double PrimitivePlane(CVector3 point, CVector3 centre, CVector3 normal)
{
	CVector3 plane = normal;
	plane.Normalize();
	double planeDistance = plane.Dot(point - centre);
	return planeDistance;
}

double PrimitiveInverseBox(CVector3 point, CVector3 corner1, CVector3 corner2)
{
	double distance, planeDistance;
	planeDistance = PrimitivePlane(point, corner1, CVector3(1,0,0));
	distance = planeDistance;
	planeDistance = PrimitivePlane(point, corner2, CVector3(-1,0,0));
	distance = (planeDistance < distance) ? planeDistance : distance;

	planeDistance = PrimitivePlane(point, corner1, CVector3(0,1,0));
	distance = (planeDistance < distance) ? planeDistance : distance;
	planeDistance = PrimitivePlane(point, corner2, CVector3(0,-1,0));
	distance = (planeDistance < distance) ? planeDistance : distance;

	planeDistance = PrimitivePlane(point, corner1, CVector3(0,0,1));
	distance = (planeDistance < distance) ? planeDistance : distance;
	planeDistance = PrimitivePlane(point, corner2, CVector3(0,0,-1));
	distance = (planeDistance < distance) ? planeDistance : distance;

	return distance;
}

double PrimitiveBox(CVector3 point, CVector3 corner1, CVector3 corner2)
{
	double distance, planeDistance;
	planeDistance = PrimitivePlane(point, corner1, CVector3(-1,0,0));
	distance = planeDistance;
	planeDistance = PrimitivePlane(point, corner2, CVector3(1,0,0));
	distance = (planeDistance > distance) ? planeDistance : distance;

	planeDistance = PrimitivePlane(point, corner1, CVector3(0,-1,0));
	distance = (planeDistance > distance) ? planeDistance : distance;
	planeDistance = PrimitivePlane(point, corner2, CVector3(0,1,0));
	distance = (planeDistance > distance) ? planeDistance : distance;

	planeDistance = PrimitivePlane(point, corner1, CVector3(0,0,-1));
	distance = (planeDistance > distance) ? planeDistance : distance;
	planeDistance = PrimitivePlane(point, corner2, CVector3(0,0,1));
	distance = (planeDistance > distance) ? planeDistance : distance;

	return distance;
}

double PrimitiveSphere(CVector3 point, CVector3 center, double radius)
{
	double distance = (point - center).Length() - radius;
	return distance;
}

double PrimitiveInverseSphere(CVector3 point, CVector3 center, double radius)
{
	double distance = radius - (point - center).Length();
	return distance;
}
