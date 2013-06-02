#pragma OPENCL EXTENSION cl_khr_byte_addressable_store : enable

typedef float4 cl_float4;
typedef float cl_float;
typedef int cl_int;
typedef unsigned int cl_uint;
typedef unsigned short cl_ushort;

#include "cl_data.h"

#define MAX_RAYMARCHING 500

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
	float shadow = 0.0;
	float factor = distThresh * 1000.0f;
	for(int count = 0; (count < 100); count++)
	{
		float4 pointTemp = point + lightVector*scan;
		float distance = CalculateDistance(pointTemp, fractal).distance;
		scan += distance * 2.0;
		
		if(scan > factor)
		{
			shadow = 1.0;
			break;
		}
		
		if(distance < distThresh)
		{
			shadow = scan / factor;
			break;
		}
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

float4 IndexToColour(int index, global float4 *palette)
{
	float4 colOut, col1, col2, colDiff;

	if (index < 0)
	{
		colOut = palette[255];
	}
	else
	{
		int index2 = index % 65280;
		int no = index2 / 256;
		col1 = palette[no];
		col2 = palette[no+1];
		colDiff = col2 - col1;
		float delta = (index2 % 256)/256.0;
		colOut = col1 + colDiff * delta;
	}
	colOut.w = 0.0f;
	return colOut;
}

float4 Background(float4 viewVector, sClParams *params)
{
	float4 vector = {1.0, 1.0, -1.0, 0.0};
	vector = fast_normalize(vector);
	float grad = dot(viewVector, vector) + 1.0;
	float4 colour;
	if(grad < 1.0)
	{
		float ngrad = 1.0 - grad;
		colour = params->backgroundColour3 * ngrad + params->backgroundColour2 * grad;
	}
	else
	{
		grad = grad - 1.0;
		float ngrad = 1.0 - grad;
		colour = params->backgroundColour2 * ngrad + params->backgroundColour1 * grad;
	}
	return colour;
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
		float scan, distThresh, distance;
		
		scan = 1e-10f;
				
		formulaOut outF;
		//ray-marching
		for(count = 0; count < MAX_RAYMARCHING; count++)
		{
			point = start + viewVector * scan;
			outF = CalculateDistance(point, &fractal);
			distance = outF.distance;
			distThresh = scan * resolution * params.persp;
			
			if(distance < distThresh)
			{
				found = true;
				break;
			}
					
			float step = (distance  - 0.5*distThresh) * params.DEfactor;			
			scan += step;
			
			if(scan > 50) break;
		}
		
		
		//binary searching
		float step = distThresh;
		for(int i=0; i<10; i++)
		{
			if(distance < distThresh && distance > distThresh * 0.95)
			{
				break;
			}
			else
			{
				if(distance > distThresh)
				{
					point += viewVector * step;
				}
				else if(distance < distThresh * 0.95)
				{
					point -= viewVector * step;
				}
			}
			outF = CalculateDistance(point, &fractal);
			distance = outF.distance;
			step *= 0.5;
		}
		
		float zBuff = scan;
		
		float4 colour = 0.0f;
		if(found)
		{
			float4 normal = NormalVector(&fractal, point, distance, distThresh);
			
			float4 lightVector = (float4) {
				cos(params.mainLightAlfa - 0.5 * M_PI) * cos(-params.mainLightBeta), 
				sin(params.mainLightAlfa - 0.5 * M_PI) * cos(-params.mainLightBeta), 
				sin(-params.mainLightBeta), 
				0.0f};
			lightVector = Matrix33MulFloat3(rot, lightVector);
			float shade = dot(lightVector, normal);
			if(shade<0) shade = 0;
			
			float shadow = Shadow(&fractal, point, lightVector, distThresh);
			//float shadow = 1.0f;
			//shadow = 0.0;
			
			float4 half = lightVector - viewVector;
			half = fast_normalize(half);
			float specular = dot(normal, half);
			if (specular < 0.0f) specular = 0.0f;
			specular = pown(specular, 30.0);
			if (specular > 15.0) specular = 15.0;
			
			float ao = FastAmbientOcclusion(&fractal, point, normal, distThresh, 1.0f, 3);
			
			int colourNumber = outF.colourIndex * params.colouringSpeed + 256.0 * params.colouringOffset;
			float4 surfaceColour = 1.0;
			if (params.colouringEnabled) surfaceColour = IndexToColour(colourNumber, inBuff->palette);
			
			colour = (shade * surfaceColour + specular * params.specularIntensity) * shadow * params.mainLightIntensity + ao * surfaceColour * params.ambientOcclusionIntensity;
			colour.w = 0;
		}
		else
		{
			colour = Background(viewVector, &params);
		}
		
		float glow = count / 2560.0f;
		float glowN = 1.0f - glow;
		if(glowN < 0.0f) glowN = 0.0f;
		float4 glowColor;
		glowColor.x = 1.0f * glowN + 1.0f * glow;
		glowColor.y = 0.0f * glowN + 1.0f * glow;
		glowColor.z = 0.0f * glowN + 0.0f * glow;
		colour += glowColor * glow;
		
		
		ushort R = convert_ushort_sat(colour.x * 65536.0f);
		ushort G = convert_ushort_sat(colour.y * 65536.0f);
		ushort B = convert_ushort_sat(colour.z * 65536.0f);
		
		out[buffIndex].R = R;
		out[buffIndex].G = G;
		out[buffIndex].B = B;
		out[buffIndex].zBuffer = zBuff;
	}
}

