/*
 * primitives.cpp
 *
 *  Created on: 15-08-2011
 *      Author: krzysztof marczak
 */

#include "primitives.h"
#include <math.h>

double PrimitivePlane(CVector3 point, CVector3 centre, CVector3 normal)
{
	CVector3 plane = normal;
	plane.Normalize();
	double planeDistance = plane.Dot(point - centre);
	return planeDistance;
}

double PrimitiveInvertedBox(CVector3 point, CVector3 center, CVector3 size)
{
	double distance, planeDistance;
	CVector3 corner1(center.x - 0.5*size.x, center.y - 0.5*size.y, center.z - 0.5*size.z);
	CVector3 corner2(center.x + 0.5*size.x, center.y + 0.5*size.y, center.z + 0.5*size.z);

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

double PrimitiveBox(CVector3 point, CVector3 center, CVector3 size)
{
	double distance, planeDistance;
	CVector3 corner1(center.x - 0.5*size.x, center.y - 0.5*size.y, center.z - 0.5*size.z);
	CVector3 corner2(center.x + 0.5*size.x, center.y + 0.5*size.y, center.z + 0.5*size.z);

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

double PrimitiveInvertedSphere(CVector3 point, CVector3 center, double radius)
{
	double distance = radius - (point - center).Length();
	return distance;
}

double PrimitiveWater(CVector3 point, double height, double amplitude, double length, double rotation, int iterations)
{
	CRotationMatrix rotMatrix;
	rotMatrix.RotateZ(rotation/180*M_PI);
	point = rotMatrix.RotateVector(point);

	CVector3 plane(0,0,-1);
	CVector3 centre(0,0,height);
	plane.Normalize();
	double planeDistance = plane.Dot(point - centre);

	double k=0.23;
	double waveXtemp = point.x;
	double waveYtemp = point.y;
	double waveX = 0;
	double waveY = 0;
	double p = 1;
	for(int i=1; i<=iterations; i++)
	{
		waveXtemp = sin(i + 0.4*(waveX)*p + sin(k* point.y / length*p) + point.x/length*p)/p;
		waveYtemp = cos(i + 0.4*(waveY)*p + sin(point.x / length*p) + k*point.y/length*p)/p;
		waveX+=waveXtemp;
		waveY+=waveYtemp;
		p *= 1.872;
	}

	planeDistance += (waveX + waveY) * amplitude;

	return planeDistance;
}
