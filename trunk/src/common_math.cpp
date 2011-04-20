/********************************************************
/                   MANDELBULBER                        *
/                                                       *
/ author: Krzysztof Marczak                             *
/ contact: buddhi1980@gmail.com                         *
/ licence: GNU GPL                                      *
********************************************************/

/*
 * common_math.cpp
 *
 *  Created on: 2010-01-23
 *      Author: krzysztof
 */
#include "common_math.h"
#include <cstdlib>

//********** Random ******************************
int Random(int max)
{
#ifdef WIN32
	return (rand()+rand()*32768) % (max + 1);
#else
	return rand() % (max + 1);
#endif
}

double dMax(double a, double b, double c)
{
	if(a > b) {
		if (a > c)
			return a;
		return c;
	}
	if (b > c)
		return b;
	return c;
}

void QuickSortZBuffer(sSortZ *dane, int l, int p)
{
  int i,j; //robocze indeksy
  sSortZ x,y; //robocze zmienne typu sortowany obiekt

  i=l;           //{ i : = minimalny klucz }
  j=p;           //{ j : = maksymalny klucz }
  x=dane[(l+p)/2];  //{ x - wyraz srodkowy sortowanej tablicy }
  do
  {             //{ cala historie bedziemy powtarzac tak dlugo jak . . . }
	while(dane[i].z < x.z)              //{ dopoki jestesmy na lewo od srodkowego }
	{
	  i++;                       //{ ...powiekszamy indeks i }
	}
	while(x.z < dane[j].z)                  //{ dopoki na prawo od srodkowego }
	{
	  j--;                       //{ ...zmniejszamy indeks j }
	}
	if(i<=j)                        //{ jezeli i <= j wtedy : }
	{                               //{ zamieniamy miejscami wyrazy i, j }
	  y=dane[i];                      //{ podstawienie do zmiennej roboczej }
	  dane[i]=dane[j];                    //{ pierwszy etap zamiany }
	  dane[j]=y;                      //{ drugi etap zamiany }
	  i++;                       //{ zwiekszenie indeksu i }
	  j--;                       //{ zmniejszenie indeksu j }
	}
  }
  while(i<=j);      //{ nie zostanie spelniony warunek i>j }
  if(l<j) QuickSortZBuffer(dane, l, j);
  if(i<p) QuickSortZBuffer(dane, i, p);
}

CVector3 Projection3D(CVector3 point, CVector3 vp, CRotationMatrix mRot, enumPerspectiveType perspectiveType, double fov, double zoom)
{
	double perspFactor = 1.0 + point.y * fov;
	CVector3 vector1, vector2;

	if (perspectiveType == fishEye)
	{
		vector1.x = sin(fov * point.x) * point.y;
		vector1.z = sin(fov * point.z) * point.y;
		vector1.y = cos(fov * point.x) * cos(fov * point.z) * point.y;

	}
	else if(perspectiveType == equirectangular)
	{
		vector1.x = sin(fov * point.x) * cos(fov * point.z) * point.y;
		vector1.z = sin(fov * point.z) * point.y;
		vector1.y = cos(fov * point.x) * cos(fov * point.z) * point.y;
	}
	else //tree-point perspective
	{
		vector1.x = point.x * perspFactor;
		vector1.y = point.y * zoom;
		vector1.z = point.z * perspFactor;
	}

	vector2 = mRot.RotateVector(vector1);

	CVector3 result = vector2 + vp;
	return result;
}

