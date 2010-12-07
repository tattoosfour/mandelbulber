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
