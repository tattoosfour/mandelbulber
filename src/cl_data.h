/*
 * cl_data.h
 *
 *  Created on: 18-09-2011
 *      Author: krzysztof
 */

typedef struct
{
	unsigned int N;
	float power;
	int formula;
	float mandelboxScale;
} sClFractal;

typedef struct
{
	int width;
	int height;
	float alpha;
	float beta;
	float gamma;
	float zoom;
	float persp;
	float4 vp;
} sClParams;

