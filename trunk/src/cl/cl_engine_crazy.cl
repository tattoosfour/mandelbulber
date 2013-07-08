#pragma OPENCL EXTENSION cl_khr_byte_addressable_store : enable

typedef float4 cl_float4;
typedef float cl_float;
typedef int cl_int;
typedef unsigned int cl_uint;
typedef unsigned short cl_ushort;

#include "cl_data.h"

typedef struct
{
	float4 z;
	float iters;
	float distance;
	float colourIndex;
} formulaOut;

static formulaOut Fractal(float4 point, sClFractal *fr);
static formulaOut CalculateDistance(float4 point, sClFractal *fractal);

inline float4 Matrix33MulFloat3(matrix33 matrix, float4 vect)
{
	float4 out;
	out.x = dot(vect, matrix.m1);
	out.y = dot(vect, matrix.m2);
	out.z = dot(vect, matrix.m3);
	out.w = 0.0f;
	return out;
}

matrix33 Matrix33MulMatrix33(matrix33 m1, matrix33 m2)
{
	matrix33 out;
	out.m1.x = m1.m1.x * m2.m1.x + m1.m1.y * m2.m2.x + m1.m1.z * m2.m3.x;
	out.m1.y = m1.m1.x * m2.m1.y + m1.m1.y * m2.m2.y + m1.m1.z * m2.m3.y;
	out.m1.z = m1.m1.x * m2.m1.z + m1.m1.y * m2.m2.z + m1.m1.z * m2.m3.z;
	out.m2.x = m1.m2.x * m2.m1.x + m1.m2.y * m2.m2.x + m1.m2.z * m2.m3.x;
	out.m2.y = m1.m2.x * m2.m1.y + m1.m2.y * m2.m2.y + m1.m2.z * m2.m3.y;
	out.m2.z = m1.m2.x * m2.m1.z + m1.m2.y * m2.m2.z + m1.m2.z * m2.m3.z;
	out.m3.x = m1.m3.x * m2.m1.x + m1.m3.y * m2.m2.x + m1.m3.z * m2.m3.x;
	out.m3.y = m1.m3.x * m2.m1.y + m1.m3.y * m2.m2.y + m1.m3.z * m2.m3.y;
	out.m3.z = m1.m3.x * m2.m1.z + m1.m3.y * m2.m2.z + m1.m3.z * m2.m3.z;
	return out;
}

matrix33 RotateX(matrix33 m, float angle)
{
		matrix33 out, rot;
		float s = sin(angle);
		float c = cos(angle);		
		rot.m1 = (float4) {1.0f, 0.0f, 0.0f, 0.0f};
		rot.m2 = (float4) {0.0f, c   , -s  , 0.0f};
		rot.m3 = (float4) {0.0f, s   ,  c  , 0.0f};
		out = Matrix33MulMatrix33(m, rot);
		return out;
}

matrix33 RotateY(matrix33 m, float angle)
{
		matrix33 out, rot;
		float s = sin(angle);
		float c = cos(angle);		
		rot.m1 = (float4) {c   , 0.0f, s   , 0.0f};
		rot.m2 = (float4) {0.0f, 1.0f, 0.0f, 0.0f};
		rot.m3 = (float4) {-s  , 0.0f, c   , 0.0f};
		out = Matrix33MulMatrix33(m, rot);
		return out;
}

matrix33 RotateZ(matrix33 m, float angle)
{
		matrix33 out, rot;
		float s = sin(angle);
		float c = cos(angle);		
		rot.m1 = (float4) { c  , -s  , 0.0f, 0.0f};
		rot.m2 = (float4) { s  ,  c  , 0.0f, 0.0f};
		rot.m3 = (float4) {0.0f, 0.0f, 1.0f, 0.0f};
		out = Matrix33MulMatrix33(m, rot);
		return out;
}



/*
float PrimitivePlane(float4 point, float4 centre, float4 normal)
{
	float4 plane = normal;
	plane = plane * (1.0/ fast_length(plane));
	float planeDistance = dot(plane, point - centre);
	return planeDistance;
}

float PrimitiveBox(float4 point, float4 center, float4 size)
{
	float distance, planeDistance;
	float4 corner1 = (float4){center.x - 0.5f*size.x, center.y - 0.5f*size.y, center.z - 0.5f*size.z, 0.0f};
	float4 corner2 = (float4){center.x + 0.5f*size.x, center.y + 0.5f*size.y, center.z + 0.5f*size.z, 0.0f};

	planeDistance = PrimitivePlane(point, corner1, (float4){-1,0,0,0});
	distance = planeDistance;
	planeDistance = PrimitivePlane(point, corner2, (float4){1,0,0,0});
	distance = (planeDistance > distance) ? planeDistance : distance;

	planeDistance = PrimitivePlane(point, corner1, (float4){0,-1,0,0});
	distance = (planeDistance > distance) ? planeDistance : distance;
	planeDistance = PrimitivePlane(point, corner2, (float4){0,1,0,0});
	distance = (planeDistance > distance) ? planeDistance : distance;

	planeDistance = PrimitivePlane(point, corner1, (float4){0,0,-1,0});
	distance = (planeDistance > distance) ? planeDistance : distance;
	planeDistance = PrimitivePlane(point, corner2, (float4){0,0,1,0});
	distance = (planeDistance > distance) ? planeDistance : distance;

	return distance;
}
*/

float Opacity(float step, int iters, int maxN, float trim, float opacitySp)
{
	float opacity = ((float)iters - trim) / maxN;
	if(opacity < 0.0) opacity = 0;
	opacity*=opacity;
	opacity*=step  * opacitySp;
	if(opacity > 1.0) opacity = 1.0;
	return opacity;
}

float4 NormalVector(sClFractal *fractal, float4 point, float mainDistance, float distThresh)
{
	float delta = distThresh;
	float s1 = CalculateDistance(point + (float4){delta,0.0f,0.0f,0.0f}, fractal).distance;
	float s2 = CalculateDistance(point + (float4){0.0f,delta,0.0f,0.0f}, fractal).distance;
	float s3 = CalculateDistance(point + (float4){0.0f,0.0f,delta,0.0f}, fractal).distance;
	float4 normal = (float4) {s1 - mainDistance, s2 - mainDistance, s3 - mainDistance, 1.0e-10f};
	normal = fast_normalize(normal);
	return normal;
}


float Shadow(sClFractal *fractal, float4 point, float4 lightVector, float distThresh)
{
	float scan = distThresh * 2.0f;
	float shadow = 1.0;
	float factor = distThresh * 500.0f;
	float last_distance = distThresh;
	int N = fractal->N;
	float opacitySp = fractal->opacity;
	float opacityTrim = fractal->opacityTrim;
	for(int count = 0; (count < 100); count++)
	{
		float4 pointTemp = point + lightVector*scan;
		formulaOut out = CalculateDistance(pointTemp, fractal);
		
		float step = out.distance;
		if(step < distThresh) step = distThresh;
		scan += step;
		
		if(scan > factor)
		{
			break;
		}
		
		float opacity = Opacity(last_distance, out.iters, N, opacityTrim, opacitySp);

		shadow-=opacity * (factor - scan) / factor;
		
		if(out.iters == N || shadow < 0.0)
		{
			shadow-= (factor - scan) / factor;
			if(shadow < 0) shadow = 0;
			break;
		}
		last_distance = step;
	}
	return shadow;
}


float FastAmbientOcclusion(sClFractal *fractal, float4 point, float4 normal, float dist_thresh, float tune, int quality)
{
	//reference: http://www.iquilezles.org/www/material/nvscene2008/rwwtt.pdf (Iñigo Quilez – iq/rgba)

	float delta = dist_thresh;
	float aoTemp = 0;
	for(int i=1; i<quality*quality; i++)
	{
		float scan = i * i * delta;
		float4 pointTemp = point + normal * scan;
		float dist = CalculateDistance(pointTemp, fractal).distance;
		aoTemp += 1.0/(native_powr(2.0,(float)i)) * (scan - tune*dist)/dist_thresh;
	}
	float ao = 1.0 - 0.2 * aoTemp;
	if(ao < 0) ao = 0;
	return ao;
}

float4 AmbientOcclusion(sClFractal *fractal, float4 point, float dist_thresh, int noOfVectors, global sClInBuff *inBuff)
{
	float4 AO = 0.0;
	int count = 0;
	float factor = dist_thresh * 500.0f;
	float last_distance = dist_thresh;
	int N = fractal->N;
	float opacitySp = fractal->opacity;
	float opacityTrim = fractal->opacityTrim;
	
	for (int i = 0; i < noOfVectors; i++)
	{
		float shadow = 1.0;
		float scan = dist_thresh * 2.0f;

		float4 d = inBuff->vectorsAround[i];
		float4 colour = inBuff->vectorsAroundColours[i];
		for (int count = 0; (count < 100); count++)
		{
			
			float4 pointTemp = point + d * scan;
			formulaOut out = CalculateDistance(pointTemp, fractal);
			
			float step = out.distance * 5.0;
			if(step < dist_thresh * 5.0) step = dist_thresh * 5.0;
			scan += step;
						
			if (scan > factor)
			{
				break;
			}

			float opacity = Opacity(last_distance, out.iters, N, opacityTrim, opacitySp);
			shadow-=opacity * (factor - scan) / factor;
			
			if(out.iters == N || shadow < 0.0)
			{
				shadow -= (factor - scan) / factor;
				if(shadow < 0.0) shadow = 0.0;
				break;
			}
			last_distance = step;
		}

		AO += shadow * colour;
	}

	AO /= noOfVectors;
	return AO;
}


//------------------ MAIN RENDER FUNCTION --------------------
kernel void fractal3D(global sClPixel *out, global sClInBuff *inBuff, sClParams Gparams, sClFractal Gfractal, int Gcl_offset)
{
	sClParams params = Gparams;
	sClFractal fractal = Gfractal;
	int cl_offset = Gcl_offset;
	
	const unsigned int i = get_global_id(0) + cl_offset;
	const unsigned int imageX = i % params.width;
	const unsigned int imageY = i / params.width;
	const unsigned int buffIndex = (i - cl_offset);
	
	if(imageY < params.height)
	{
		float2 screenPoint = (float2) {convert_float(imageX), convert_float(imageY)};
		float width = convert_float(params.width);
		float height = convert_float(params.height);
		float resolution = 1.0f/width;
		
		const float4 one = (float4) {1.0f, 0.0f, 0.0f, 0.0f};
		const float4 ones = 1.0f;
		
		matrix33 rot;
		rot.m1 = (float4){1.0f, 0.0f, 0.0f, 0.0f};
		rot.m2 = (float4){0.0f, 1.0f, 0.0f, 0.0f};
		rot.m3 = (float4){0.0f, 0.0f, 1.0f, 0.0f};
		rot = RotateZ(rot, params.alpha);
		rot = RotateX(rot, params.beta);
		rot = RotateY(rot, params.gamma);
		
		float4 back = (float4) {0.0f, 1.0f, 0.0f, 0.0f} / params.persp * params.zoom;
		float4 start = params.vp - Matrix33MulFloat3(rot, back);
		
		float aspectRatio = width / height;
		float x2,z2;
		x2 = (screenPoint.x / width - 0.5) * aspectRatio;
		z2 = (screenPoint.y / height - 0.5);
		float4 viewVector = (float4) {x2 * params.persp, 1.0f, z2 * params.persp, 0.0f}; 
		viewVector = Matrix33MulFloat3(rot, viewVector);
		
		bool found = false;
		int count;
		
		float4 point;
		float scan, distThresh, distance, step;
		
		scan = 1e-10f;
		//ray-marching
		for(count = 0; count < 2000; count++)
		{
			point = start + viewVector * scan;
			formulaOut out = CalculateDistance(point, &fractal);
			distThresh = scan * resolution * params.persp;
			
			if(out.iters == fractal.N)
			{
				found = true;
				break;
			}
			
			if(out.distance * params.DEfactor > distThresh)
			{
				step = out.distance * params.DEfactor;
			}
			else
			{
				step = distThresh * params.DEfactor;
			}
			
			scan += step;
			
			if(scan > 10.0) break;
		}

		//binary searching
		scan-=step;
		//step = distThresh*0.5;
		for(int i=0; i<10; i++)
		{
			formulaOut out = Fractal(point, &fractal);
			if(out.iters < fractal.N)
			{
				scan += step;
				point = start + viewVector * scan;
			}
			else
			{
				scan -= step;
				point = start + viewVector * scan;
			}
			step *= 0.5;
		}
		
			
		float zBuff = scan / params.zoom - 1.0;
		
		float4 colour = {0.0f, 0.0f, 0.0f, 0.0f};;
		bool first = true;
		
		float4 lightVector = (float4) {-0.7f, -0.7f, -0.7f, 0.0f};
		lightVector = Matrix33MulFloat3(rot, lightVector);
		
		for(;scan>1e-10; scan -= step)
		{			
			point = start + viewVector * scan;
			formulaOut out = CalculateDistance(point, &fractal);
			int iters = out.iters;
			float opacity = Opacity(step, iters, fractal.N, fractal.opacityTrim, fractal.opacity);
			if(first && found) opacity = 1.0;
			
			float shade = 1.0;
			float shadow = 0.0;
			float4 ao = 0.5;
			
			if(opacity > 0.0)
			{
				//if(first)
				//{
					//float4 normal = NormalVector(&fractal, point, out.distance, distThresh);
					//shade = dot(lightVector, normal);
					//if(shade<0) shade = 0;
				//}
				//else
				//{
				//	shade = 1.0;
				//}

				shadow = Shadow(&fractal, point, lightVector, distThresh);
				ao = AmbientOcclusion(&fractal, point, distThresh, params.AmbientOcclusionNoOfVectors, inBuff) * 2.0;
			}

			float4 newcolour = 0.5*shade * shadow + ao;
			colour = colour * (1.0 - opacity) + newcolour * opacity;
			
			distThresh = scan * resolution * params.persp;

			step = out.distance * 0.2;
			
			if(out.distance * 0.5 < distThresh * 0.5)	step = distThresh * 0.5;

			first = false;
		}
		
		ushort R = convert_ushort_sat(colour.x * 65536.0f);
		ushort G = convert_ushort_sat(colour.y * 65535.0f);
		ushort B = convert_ushort_sat(colour.z * 65535.0f);
		
		out[buffIndex].R = R;
		out[buffIndex].G = G;
		out[buffIndex].B = B;
		out[buffIndex].zBuffer = zBuff;
	}
}

