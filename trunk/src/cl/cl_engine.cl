#pragma OPENCL EXTENSION cl_khr_byte_addressable_store : enable

typedef float4 cl_float4;
typedef float cl_float;
typedef int cl_int;
typedef unsigned int cl_uint;

#include "cl_data.h"

static float Fractal(float4 point, sClFractal *fr);

/*
typedef struct
{
	float2 m1;
	float2 m2;
} matrix22;

matrix22 RotationMatrix22(float alpha)
{
	matrix22 matrix;
	matrix.m1 = (float2) {cos(alpha), sin(alpha)};
	matrix.m2 = (float2) {-sin(alpha), cos(alpha)};
	return matrix;
}

float2 Matrix22MulFloat2(matrix22 matrix, float2 vect)
{
	float2 out;
	out.x = dot(vect, matrix.m1);
	out.y = dot(vect, matrix.m2);
	return out;
}
*/

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
float4 Projection3D(float4 point, float4 vp, matrix33 mRot, float fov, float zoom)
{
	float perspFactor = 1.0 + point.y * fov;
	float4 vector1, vector2;
	
	vector1.x = point.x * perspFactor;
	vector1.y = point.y * zoom;
	vector1.z = point.z * perspFactor;

	vector1.w = 0;
	
	vector2 = Matrix33MulFloat3(mRot, vector1);
	vector2.w = 0;

	float4 result = vector2 + vp;
	return result;
}
*/
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
//--------------- formulas ----------------

/*
float Fractal(float4 point, sClFractal *fr)
{
	float distance = 0;
	int N = fr->N;
	float power = fr->power;
	float4 z = point;
	float r = fast_length(z);
	float r_dz = 1.0f;
	
	for(int i=0; i<N; i++)
	{
		float th0 = asin(native_divide(z.z ,r));
		float ph0 = atan2(z.y, z.x);
		float rp = native_powr(r, power - 1.0f);
		float th = th0 * power;
		float ph = ph0 * power;
		float cth = native_cos(th);
		r_dz = rp * r_dz * power + 1.0f;
		rp *= r;
		z = (float4) {cth * native_cos(ph), cth * native_sin(ph), native_sin(th), 0.0f};
		z*=rp;
		z+=point;
		r = fast_length(z);
		if(r>4.0f) 
		{
			distance = 0.5f * r * native_log(r) / (r_dz);
			break;
		}
	}
	if(distance < 0) distance = 0;
	return distance;
}
*/
/*
float Mandelbulb2(float4 point, float power, int N)
{
	float distance = 0.0f;
	float4 one = {1.0f, 0.0f, 0.0f, 0.0f};
	float4 z = point;
	float4 dz = 0.0f;
	float ph_dz = 0;
	float th_dz = 0;
	float r = fast_length(z);
	float r_dz = 1.0f;

	for(int i=0; i<N; i++)
	{
		float r1 = native_powr(r, power - 1.0f);
		float r2 = r1 * r;
		float th = atan2(z.y, z.x);
		float ph = -atan2(z.z, sqrt(z.x * z.x + z.y * z.y));

		float p_r1_rdz = power * r1 * r_dz;
		float ph_phdz = (power - 1.0f) * ph + ph_dz;
		float th_thdz = (power - 1.0f) * th + th_dz;
		dz.x = native_cos(ph_phdz) * native_cos(th_thdz);
		dz.y = native_cos(ph_phdz) * native_sin(th_thdz);
		dz.z = native_sin(ph_phdz);
		dz = dz * p_r1_rdz + one;
		r_dz = fast_length(dz);
		th_dz = atan2(dz.y, dz.x);
		ph_dz = -atan2(dz.z, sqrt(dz.x * dz.x + dz.y * dz.y));

		float pth = power * th;
		float pph = power * ph;
		z.x = native_cos(pph) * native_cos(pth);
		z.y = native_cos(pph) * native_sin(pth);
		z.z = native_sin(pph);

		z = z * r2 + point;
		r = fast_length(z);
		
		if(r>4.0f) 
		{
			distance = 0.5f * r * native_log(r) / (r_dz);
			break;
		}
	}
	if(distance < 0) distance = 0;
	return distance;
}
*/

/*
float Fractal(float4 point, sClFractal *fr)
{
	float distance = 0;
	int N = fr->N;
	float4 z = point;
	float tgladDE = 1.0f;
	float4 ones = 1.0f;
	float foldingLimit = fr->mandelbox.foldingLimit;
	float foldingValue = fr->mandelbox.foldingValue;
	float mr2 = fr->mandelbox.minRadius * fr->mandelbox.minRadius;
	float fr2 = fr->mandelbox.fixedRadius * fr->mandelbox.fixedRadius;
	float scale = fr->mandelbox.scale;
	
	for(int i=0; i<N; i++)
	{
		int4 cond1, cond2;
		
		cond1.x = isgreater(z.x, foldingLimit);
		cond1.y = isgreater(z.y, foldingLimit);
		cond1.z = isgreater(z.z, foldingLimit);
		cond2.x = isless(z.x, -foldingLimit);
		cond2.y = isless(z.y, -foldingLimit);
		cond2.z = isless(z.z, -foldingLimit);
		z.x = select(z.x,  foldingValue - z.x, cond1.x);
		z.y = select(z.y,  foldingValue - z.y, cond1.y);
		z.z = select(z.z,  foldingValue - z.z, cond1.z);
		z.x = select(z.x,  -foldingValue - z.x, cond2.x);
		z.y = select(z.y,  -foldingValue - z.y, cond2.y);
		z.z = select(z.z,  -foldingValue - z.z, cond2.z);
		
		//z = fabs(z + ones) - fabs(z - ones) - z;

		float rr = dot(z,z);
		float m = scale;
		if (rr < mr2)	m = native_divide(scale, mr2);
		else if (rr < fr2)	m = native_divide(scale,rr);
		
		z = Matrix33MulFloat3(fr->mandelbox.mainRot, z);
		
		z = z * m + point;
		tgladDE = tgladDE * fabs(m) + 1.0f;
		float r = dot(z,z);
		
		if(r>1024.0f) 
		{
			distance = native_sqrt(r) / fabs(tgladDE);
			break;
		}
	}
	return distance;
}
*/

/*
float MengerSponge(float4 point, int N)
{
	float4 z = point;
	float temp = 0;
	float tgladDE = 1.0f;
	float distance = 0;
	
	for (int i = 0; i < N; i++)
	{
		z = fabs(z);
		
		if (z.x - z.y < 0)
		{
			temp = z.y;
			z.y = z.x;
			z.x = temp;
		}
		if (z.x - z.z < 0)
		{
			temp = z.z;
			z.z = z.x;
			z.x = temp;
		}
		if (z.y - z.z < 0)
		{
			temp = z.z;
			z.z = z.y;
			z.y = temp;
		}
		
		z *= 3.0;

		z.x -= 2.0;
		z.y -= 2.0;
		if (z.z > 1.0) z.z -= 2.0;
		float r = fast_length(z);
		tgladDE *= 3.0;

		if (r > 1024.0f)
		{
			distance = r / fabs(tgladDE);
			break;
		}
	}
	return distance;
}
*/
//--------------------------------------------

float CalculateDistance(float4 point, sClFractal *fractal)
{
	float distance;
	distance = Fractal(point, fractal);
	
	/*
	switch(fractal->formula)
	{
		case 1:
		{
			//distance = Mandelbulb2(point, fractal->power, fractal->N);
			break;
		}
		case 2:
		{
			//distance = Mandelbulb(point, fractal->power, fractal->N);
			break;
		}
		case 7:
		{
			//distance = MengerSponge(point, fractal->N);
			break;
		}
		case 8:
		{
			distance = Mandelbox(point, fractal);
			break;
		}
	}
	*/
	if(distance<0.0f) distance = 0.0f;
	if(distance>10.0f) distance = 10.0f;
	return distance;
}

float4 NormalVector(sClFractal *fractal, float4 point, float mainDistance, float distThresh)
{
	float delta = distThresh;
	float s1 = CalculateDistance(point + (float4){delta,0.0f,0.0f,0.0f}, fractal);
	float s2 = CalculateDistance(point + (float4){0.0f,delta,0.0f,0.0f}, fractal);
	float s3 = CalculateDistance(point + (float4){0.0f,0.0f,delta,0.0f}, fractal);
	float4 normal = (float4) {s1 - mainDistance, s2 - mainDistance, s3 - mainDistance, 1.0e-10f};
	normal = fast_normalize(normal);
	return normal;
}


float Shadow(sClFractal *fractal, float4 point, float4 lightVector, float distThresh)
{
	float scan = distThresh * 2.0f;
	float shadow = 0.0;
	float factor = distThresh * 500.0f;
	for(int count = 0; (count < 100); count++)
	{
		float4 pointTemp = point + lightVector*scan;
		float distance = CalculateDistance(pointTemp, fractal);
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
		float dist = CalculateDistance(pointTemp, fractal);
		aoTemp += 1.0/(native_powr(2.0,i)) * (scan - tune*dist)/dist_thresh;
	}
	float ao = 1.0 - 0.2 * aoTemp;
	if(ao < 0) ao = 0;
	return ao;
}


//------------------ MAIN RENDER FUNCTION --------------------
kernel void fractal3D(global char *out, sClParams params, sClFractal fractal, int line)
{
	const unsigned int i = get_global_id(0);
	const unsigned int imageX = i % params.width;
	const unsigned int imageY = i / params.width;
	const unsigned int buffIndex = i*3;
	
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
		distance = CalculateDistance(point, &fractal);
		distThresh = scan * resolution * params.persp;
		
		if(distance < distThresh)
		{
			found = true;
			break;
	  }
				
		float step = distance * params.DEfactor - 0.5*distThresh;
		scan += step;
		
		if(scan > 50) break;
	}
	
	float4 colour = 0.0f;
	if(found)
	{
		float4 normal = NormalVector(&fractal, point, distance, distThresh);
		
		float4 lightVector = (float4) {-0.7f, -0.7f, -0.7f, 0.0f};
		lightVector = Matrix33MulFloat3(rot, lightVector);
		float shade = dot(lightVector, normal);
		if(shade<0) shade = 0;
		
		float shadow = Shadow(&fractal, point, lightVector, distThresh);
		//float shadow = 1.0f;
		
		float4 half = lightVector - viewVector;
		half = fast_normalize(half);
		float specular = dot(normal, half);
		if (specular < 0.0f) specular = 0.0f;
		specular = pown(specular, 20);
		if (specular > 15.0) specular = 15.0;
		
		float ao = FastAmbientOcclusion(&fractal, point, normal, distThresh, 0.8f, 3);
		//float ao = 0.0f;
		
		colour.x = (shade + specular) * shadow + ao;
		colour.y = (shade + specular) * shadow + ao;
		colour.z = (shade + specular) * shadow + ao;
	}
	
	float glow = count / 128.0f;
	float glowN = 1.0f - glow;
	if(glowN < 0.0f) glowN = 0.0f;
	float4 glowColor;
	glowColor.x = 1.0f * glowN + 1.0f * glow;
	glowColor.y = 0.0f * glowN + 1.0f * glow;
	glowColor.z = 0.0f * glowN + 0.0f * glow;
	colour += glowColor * glow;
	
	
	uchar R = convert_uchar_sat(colour.x * 150.0f);
	uchar G = convert_uchar_sat(colour.y * 150.0f);
	uchar B = convert_uchar_sat(colour.z * 150.0f);
	
	out[buffIndex] = R;
	out[buffIndex+1] = G;
	out[buffIndex+2] = B;
}

