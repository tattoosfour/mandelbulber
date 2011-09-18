#pragma OPENCL EXTENSION cl_khr_byte_addressable_store : enable

#include "cl_data.h"

typedef struct
{
	float2 m1;
	float2 m2;
} matrix22;

typedef struct
{
	float4 m1;
	float4 m2;
	float4 m3;
} matrix33;

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

float4 Matrix33MulFloat3(matrix33 matrix, float4 vect)
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

float PrimitivePlane(float4 point, float4 centre, float4 normal)
{
	float4 plane = normal;
	plane = plane * (1.0/ length(plane));
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

float Mandelbulb(float4 point, float power, int N)
{
	float distance = 0;
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
		if(r>100.0f) 
		{
			distance = 0.5f * r * native_log(r) / (r_dz);
			break;
		}
	}
	if(distance < 0) distance = 0;
	return distance;
}

float Mandelbox(float4 point, float scale, int N)
{
	float distance = 0;
	float4 z = point;
	float tgladDE = 1.0f;
	for(int i=0; i<N; i++)
	{
		float4 one = 1.0f;
		z = fabs(z + one) - fabs(z - one) - z;
		float rr = dot(z,z);
		float m = scale;
		if (rr < 0.25f)	m = scale * 4.0f;
		else if (rr < 1.0f)	m = native_divide(scale,rr);
		
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

float CalculateDistance(float4 point, sClFractal *fractal)
{
	//float distance = Mandelbox(point, -2.0f, fractal->N);
	float distance = Mandelbulb(point, fractal->power, fractal->N);
	//return PrimitiveBox(point, (float4) {0,0,0,0}, (float4){1.0,1.0,1.0,1.0});
	//return length(point) - 3.0;
	
	if(distance<0) distance = 0;
	if(distance>10) distance = 10;
	return distance;
}

float4 NormalVector(sClFractal *fractal, float4 point, float mainDistance, float distThresh)
{
	float delta = distThresh * 0.1;
	float s1 = CalculateDistance(point + (float4){delta,0.0f,0.0f,0.0f}, fractal);
	float s2 = CalculateDistance(point + (float4){0.0f,delta,0.0f,0.0f}, fractal);
	float s3 = CalculateDistance(point + (float4){0.0f,0.0f,delta,0.0f}, fractal);
	float4 normal = (float4) {s1 - mainDistance, s2 - mainDistance, s3 - mainDistance, 1e-10f};
	normal = fast_normalize(normal);
	return normal;
}

float Shadow(sClFractal *fractal, float4 point, float4 lightVector, float distThresh)
{
	float scan = distThresh * 2.0;
	float shadow = 0;
	float factor = distThresh * 1000.0f;
	for(int count = 0; (count < 100); count++)
	{
		float4 pointTemp = point + lightVector*scan;
		float distance = CalculateDistance(pointTemp, fractal);
		scan += distance;
		
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
		aoTemp += 1.0/(pow(2.0,i)) * (scan - tune*dist)/dist_thresh;
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
	for(count = 0; (count < 1000); count++)
	{
		point = start + viewVector * scan;
		distance = CalculateDistance(point, &fractal);
		distThresh = scan * resolution * params.persp;
		
		if(distance < distThresh)
		{
			found = true;
			break;
	  }
				
		float step = distance * 0.8f;
		scan += step;
		
		if(scan > 50) break;
	}
	
	float4 colour = 0.5f;
	if(found)
	{
		float4 normal = NormalVector(&fractal, point, distance, distThresh);
		
		float4 lightVector = (float4) {-0.7f, -0.7f, -0.7f, 0.0f};
		lightVector = Matrix33MulFloat3(rot, lightVector);
		float shade = dot(lightVector, normal);
		if(shade<0) shade = 0;
		
		float shadow = Shadow(&fractal, point, lightVector, distThresh);
		
		float4 half = lightVector - viewVector;
		half = fast_normalize(half);
		float specular = dot(normal, half);
		if (specular < 0.0f) specular = 0.0f;
		specular = pown(specular, 20);
		if (specular > 15.0) specular = 15.0;
		
		float ao = FastAmbientOcclusion(&fractal, point, normal, distThresh, 1.0f, 3);
		colour.x = (shade + specular) * shadow + ao;
		colour.y = (shade + specular) * shadow + ao;
		colour.z = (shade + specular) * shadow + ao;
	}
	
	uchar R = convert_uchar_sat(colour.x * 150);
	uchar G = convert_uchar_sat(colour.y * 150);
	uchar B = convert_uchar_sat(colour.z * 150);
	
	out[buffIndex] = R;
	out[buffIndex+1] = G;
	out[buffIndex+2] = B;
}
