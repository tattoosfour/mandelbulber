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
#include <math.h>
#include "common_math.h"
#include <stdlib.h>

//********** Random ******************************
int Random(int max)
{
#ifdef WIN32
	return (rand()+rand()*32768) % (max + 1);
#else
	return rand() % (max + 1);
#endif
}

//********** CosInterpolation ********************
double CosInterpolation(double x1, double x2, int res, int n)
{
	double delta_x = x2 - x1;
	double t = (double) n / res;
	double x = ((1.0 - cos(t * M_PI)) * 0.5) * delta_x + x1;
	return x;
}

//********** abs *************
//int abs(int v)
//{
//	return v >= 0 ? v : -v;
//}

//************* Macierz x Wektor *********************************
void MacierzXVector(sVector &v1, sVector &v2, sMacierz33 &m)
{
	v2.x = v1.x * m.m00 + v1.y * m.m01 + v1.z * m.m02;
	v2.y = v1.x * m.m10 + v1.y * m.m11 + v1.z * m.m12;
	v2.z = v1.x * m.m20 + v1.y * m.m21 + v1.z * m.m22;

}

double dMax (double a, double b)
{
	if(a>=b) return a; else return b;
}

double dMin (double a, double b)
{
	if(a>=b) return b; else return a;
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
