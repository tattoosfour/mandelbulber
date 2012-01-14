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
		for(count = 0; (count < 255); count++)
		{
			point = start + viewVector * scan;
			distance = CalculateDistance(point, &fractal).distance;
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
		
		float zBuff = scan / params.zoom - 1.0;
		
		float4 colour = 0.0f;
		if(found)
		{
			float4 normal = NormalVector(&fractal, point, distance, distThresh);
			
			float4 lightVector = (float4) {-0.7f, -0.7f, -0.7f, 0.0f};
			lightVector = Matrix33MulFloat3(rot, lightVector);
			float shade = dot(lightVector, normal);
			if(shade<0) shade = 0;
						
			float4 half = lightVector - viewVector;
			half = fast_normalize(half);
			float specular = dot(normal, half);
			if (specular < 0.0f) specular = 0.0f;
			specular = pown(specular, 20);
			if (specular > 15.0) specular = 15.0;
			
			colour.x = (shade + specular); 
			colour.y = (shade + specular); 
			colour.z = (shade + specular);
		}
		
		float glow = count / 128.0f;
		float glowN = 1.0f - glow;
		if(glowN < 0.0f) glowN = 0.0f;
		float4 glowColor;
		glowColor.x = 1.0f * glowN + 1.0f * glow;
		glowColor.y = 0.0f * glowN + 1.0f * glow;
		glowColor.z = 0.0f * glowN + 0.0f * glow;
		colour += glowColor * glow;
		
		
		ushort R = convert_ushort_sat(colour.x * 38400.0f);
		ushort G = convert_ushort_sat(colour.y * 38400.0f);
		ushort B = convert_ushort_sat(colour.z * 38400.0f);
		
		out[buffIndex].R = R;
		out[buffIndex].G = G;
		out[buffIndex].B = B;
		out[buffIndex].zBuffer = zBuff;
	}
}

