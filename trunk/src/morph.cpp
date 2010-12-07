/*
 * morph.cpp
 *
 *  Created on: 2010-05-30
 *      Author: krzysztof
 */

#include "morph.hpp"
#include <stdio.h>

CMorph::CMorph(int size, int recordSize)
{
	count = size;
	dataSize = recordSize;
	dataSets = new double*[count];
	for (int i = 0; i < count; i++)
	{
		dataSets[i] = new double[dataSize];
	}
	output = new double[dataSize];
}

CMorph::~CMorph()
{
	for (int i = 0; i < count; i++)
	{
		delete[] dataSets[i];
	}
	delete[] dataSets;
	delete[] output;
}

void CMorph::AddData(int index, double *data)
{
	if (index < count)
	{
		for (int i = 0; i < dataSize; i++)
		{
			dataSets[index][i] = data[i];
		}
	}
}

void CMorph::Linear(int frame, double *destData)
{
	int key = frame / framesPerKey;
	double factor = (frame % framesPerKey) / (double) framesPerKey;
	for (int i = 0; i < dataSize; i++)
	{
		double delta = dataSets[key + 1][i] - dataSets[key][i];
		output[i] = dataSets[key][i] + delta * factor;
		destData[i] = output[i];
	}
}

void CMorph::CatmullRom(int frame, double *destData)
{
	double v1, v2, v3, v4;
	int key = frame / framesPerKey;
	double factor = (frame % framesPerKey) / (double) framesPerKey;
	double factor2 = factor*factor;
	double factor3 = factor2*factor;
	for (int i = 0; i < dataSize; i++)
	{
		if (key >= 1) v1 = dataSets[key - 1][i];
		else v1 = dataSets[key][i];

		if (key < count) v2 = dataSets[key][i];
		else v2 = dataSets[count - 1][i];

		if (key < count - 1) v3 = dataSets[key + 1][i];
		else v3 = dataSets[count - 1][i];

		if (key < count - 2) v4 = dataSets[key + 2][i];
		else v4 = dataSets[count - 1][i];

		//printf("v1=%f, v2=%f, v3=%f, v4=%f\n",v1,v2,v3,v4);

		double value = 0.5 * ((2 * v2) + (-v1 + v3) * factor + (2 * v1 - 5 * v2 + 4 * v3 - v4) * factor2 + (-v1 + 3 * v2 - 3 * v3 + v4) * factor3);
		output[i]=value;
		destData[i] = output[i];
	}
}

