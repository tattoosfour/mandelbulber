#pragma OPENCL EXTENSION cl_amd_fp64 : enable

typedef float3 cl_float3;
typedef float cl_float;
typedef int cl_int;
typedef unsigned int cl_uint;
typedef unsigned short cl_ushort;

#include "cl_data.h"

#define MAX_RAYMARCHING 1000

typedef struct
{
	float3 z;
	float iters;
	float distance;
	float colourIndex;
} formulaOut;

typedef struct 
{
	float distance;
	float step;
	float3 point;
	int iters;
	float distThresh;
} sStep;

static formulaOut Fractal(float3 point, sClFractal *fr);
static formulaOut CalculateDistance(float3 point, sClFractal *fractal);

int rand(int* seed)
{
    int s = *seed;
    int i = 0;
    for(i = 0; i< 3; i++)
    {
    	int const a = 15484817;
    	int const m = 6571759; 
    	s = ((long)(s * a)) % m;
    }
    *seed = s;
    return(s % 65536);
}

inline float3 Matrix33MulFloat3(matrix33 matrix, float3 vect)
{
	float3 out;
	out.x = dot(vect, matrix.m1);
	out.y = dot(vect, matrix.m2);
	out.z = dot(vect, matrix.m3);
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
		rot.m1 = (float3) {1.0f, 0.0f, 0.0f};
		rot.m2 = (float3) {0.0f, c   , -s  };
		rot.m3 = (float3) {0.0f, s   ,  c  };
		out = Matrix33MulMatrix33(m, rot);
		return out;
}

matrix33 RotateY(matrix33 m, float angle)
{
		matrix33 out, rot;
		float s = sin(angle);
		float c = cos(angle);		
		rot.m1 = (float3) {c   , 0.0f, s   };
		rot.m2 = (float3) {0.0f, 1.0f, 0.0f};
		rot.m3 = (float3) {-s  , 0.0f, c   };
		out = Matrix33MulMatrix33(m, rot);
		return out;
}

matrix33 RotateZ(matrix33 m, float angle)
{
		matrix33 out, rot;
		float s = sin(angle);
		float c = cos(angle);		
		rot.m1 = (float3) { c  , -s  , 0.0f};
		rot.m2 = (float3) { s  ,  c  , 0.0f};
		rot.m3 = (float3) {0.0f, 0.0f, 1.0f};
		out = Matrix33MulMatrix33(m, rot);
		return out;
}



/*
float PrimitivePlane(float3 point, float3 centre, float3 normal)
{
	float3 plane = normal;
	plane = plane * (1.0/ fast_length(plane));
	float planeDistance = dot(plane, point - centre);
	return planeDistance;
}

float PrimitiveBox(float3 point, float3 center, float3 size)
{
	float distance, planeDistance;
	float3 corner1 = (float3){center.x - 0.5f*size.x, center.y - 0.5f*size.y, center.z - 0.5f*size.z};
	float3 corner2 = (float3){center.x + 0.5f*size.x, center.y + 0.5f*size.y, center.z + 0.5f*size.z};

	planeDistance = PrimitivePlane(point, corner1, (float3){-1,0,0});
	distance = planeDistance;
	planeDistance = PrimitivePlane(point, corner2, (float3){1,0,0});
	distance = (planeDistance > distance) ? planeDistance : distance;

	planeDistance = PrimitivePlane(point, corner1, (float3){0,-1,0});
	distance = (planeDistance > distance) ? planeDistance : distance;
	planeDistance = PrimitivePlane(point, corner2, (float3){0,1,0});
	distance = (planeDistance > distance) ? planeDistance : distance;

	planeDistance = PrimitivePlane(point, corner1, (float3){0,0,-1});
	distance = (planeDistance > distance) ? planeDistance : distance;
	planeDistance = PrimitivePlane(point, corner2, (float3){0,0,1});
	distance = (planeDistance > distance) ? planeDistance : distance;

	return distance;
}
*/


float3 IndexToColour(int index, global float3 *palette)
{
	float3 colOut, col1, col2, colDiff;

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
	return colOut;
}

float3 NormalVector(sClFractal *fractal, float3 point, float mainDistance, float distThresh)
{
	float delta = distThresh;
	float s1 = CalculateDistance(point + (float3){delta,0.0f,0.0f}, fractal).distance;
	float s2 = CalculateDistance(point + (float3){0.0f,delta,0.0f}, fractal).distance;
	float s3 = CalculateDistance(point + (float3){0.0f,0.0f,delta}, fractal).distance;
	float3 normal = (float3) {s1 - mainDistance, s2 - mainDistance, s3 - mainDistance};
	normal = fast_normalize(normal);
	return normal;
}


float Shadow(sClFractal *fractal, float3 point, float3 lightVector, float distThresh, float resolution)
{
	float scan = distThresh * 2.0f;
	float shadow = 0.0;
	float factor = distThresh / resolution;
	for(int count = 0; (count < 1000); count++)
	{
		float3 pointTemp = point + lightVector*scan;
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


float FastAmbientOcclusion(sClFractal *fractal, float3 point, float3 normal, float dist_thresh, float tune, int quality)
{
	//reference: http://www.iquilezles.org/www/material/nvscene2008/rwwtt.pdf (Iñigo Quilez – iq/rgba)

	float delta = dist_thresh;
	float aoTemp = 0;
	for(int i=1; i<quality*quality; i++)
	{
		float scan = i * i * delta;
		float3 pointTemp = point + normal * scan;
		float dist = CalculateDistance(pointTemp, fractal).distance;
		aoTemp += 1.0/(native_powr(2.0,(float)i)) * (scan - tune*dist)/dist_thresh;
	}
	float ao = 1.0 - 0.2 * aoTemp;
	if(ao < 0) ao = 0;
	return ao;
}

float3 AmbientOcclusion(sClFractal *fractal, float3 point, float dist_thresh, int noOfVectors, global sClInBuff *inBuff)
{
	float3 AO = 0.0;
	int count = 0;
	float factor = dist_thresh * 1000.0f;

	for (int i = 0; i < noOfVectors -1; i++)
	{
		float3 shadow = 0.0;
		float scan = dist_thresh * 2.0f;

		float3 d = inBuff->vectorsAround[i];
		float3 colour = inBuff->vectorsAroundColours[i];
		for (int count = 0; (count < 100); count++)
		{
			
			float3 pointTemp = point + d * scan;
			float distance = CalculateDistance(pointTemp, fractal).distance;
			scan += distance * 2.0;

			if (scan > factor)
			{
				shadow = colour;
				break;
			}

			if (distance < dist_thresh)
			{
				shadow = colour * scan / factor;
				break;
			}
		}

		AO += shadow;
	}

	AO /= noOfVectors;
	return AO;
}

float3 Background(float3 viewVector, sClParams *params)
{
	float3 vector = {1.0, 1.0, -1.0};
	vector = fast_normalize(vector);
	float grad = dot(viewVector, vector) + 1.0;
	float3 colour;
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

float3 VolumetricFog(sClParams *params, int buffCount, float *distanceBuff, float *stepBuff)
{
	float density = 0.0;
	float3 fogCol = params->fogColour1;
	float3 fogCol1 = fogCol;
	float3 fogCol2 = params->fogColour2;
	float3 fogCol3 = params->fogColour3;

	float colourThresh = params->fogColour1Distance;
	float colourThresh2 = params->fogColour2Distance;
	float fogReduce = params->fogDistanceFactor;
	float fogIntensity = params->fogDensity;
	for (int i = buffCount - 1; i >= 0; i--)
	{
		float densityTemp = (stepBuff[i] * fogReduce) / (distanceBuff[i] * distanceBuff[i] + fogReduce * fogReduce);

		float k = distanceBuff[i] / colourThresh;
		if (k > 1.0) k = 1.0;
		float kn = 1.0 - k;
		float3 fogTemp = (fogCol1 * kn + fogCol2 * k);

		float k2 = distanceBuff[i] / colourThresh2 * k;
		if (k2 > 1.0) k2 = 1.0;
		kn = 1.0 - k2;
		fogTemp = (fogTemp * kn + fogCol3 * k2);

		float d1 = fogIntensity * densityTemp / (1.0 + fogIntensity * densityTemp);
		float d2 = 1.0 - d1;
		fogCol = fogCol * d2 + fogTemp * d1;
		
		density += densityTemp;
	}
	density *= fogIntensity;
	//fogCol.w = density / (1.0 + density);
	return fogCol;
}

float3 RayMarching(sClParams *param, sClFractal *calcParam, float3 start, float3 direction, float maxScan, bool binaryEnable, sStep *stepBuff, int *buffCount,
		float *distThreshOut, float *lastDistOut, bool *foundOut, float *depthOut)
{
	float3 point;
	bool found = false;
	float scan = 1e-10;
	float distThresh = *distThreshOut;
	float dist = 0;
	float search_accuracy = 0.01 * param->quality;
	//printf("param->quality %f, start %f %f %f\n", param->quality, start.x, start.y, start.z);
	float search_limit = 1.0 - search_accuracy;
	float step = 1e-10;
	float resolution = 1.0f/param->width;
	*buffCount = 0;
	float distThreshInit = *distThreshOut;

	
	for (int i = 0; i < MAX_RAYMARCHING; i++)
	{
		point = start + direction * scan;
		bool max_iter = false;

		if (calcParam->constantDEThreshold)
		{
			distThresh = param->quality;
		}
		else
		{
			distThresh = scan * resolution * param->persp / param->quality + distThreshInit;
		}
		//calcParam->doubles.detailSize = distThresh;
		formulaOut outF;
		outF = CalculateDistance(point, &calcParam);
		dist = outF.distance;
		
		printf("Distance = %g\n", dist/distThresh);
		stepBuff[i].distance = dist;
		stepBuff[i].iters = outF.iters;
		stepBuff[i].distThresh = distThresh;

		if (dist > 3.0) dist = 3.0;
		if (dist < distThresh)
		{
			found = true;
			break;
		}

		stepBuff[i].step = step;
		step = (dist - 0.5 * distThresh) * param->DEfactor;
		stepBuff[i].point = point;
		(*buffCount)++;
		scan += step;
		if (scan > maxScan)
		{
			break;
		}

	}
	
	if (found && binaryEnable)
	{
		float step = distThresh * 0.5;
		for (int i = 0; i < 10; i++)
		{
			if (dist < distThresh && dist > distThresh * search_limit)
			{
				break;
			}
			else
			{
				if (dist > distThresh)
				{
					scan += step;
					point = start + direction * scan;
				}
				else if (dist < distThresh * search_limit)
				{
					scan -= step;
					point = start + direction * scan;
				}
			}
			formulaOut outF;
			outF = CalculateDistance(point, &calcParam);
					dist = outF.distance;
			//printf("Distance binary = %g\n", dist/distThresh);
			step *= 0.5;
		}
	}

	*foundOut = found;
	*distThreshOut = distThresh;
	*lastDistOut = dist;
	*depthOut = scan;
	return point;
}

//------------------ MAIN RENDER FUNCTION --------------------
kernel void fractal3D(global sClPixel *out, global sClInBuff *inBuff, int Gcl_offset)
{
	
	sClParams params = inBuff->params;
	sClFractal fractal = inBuff->fractal;
	int cl_offset = Gcl_offset;
	const unsigned int i = get_global_id(0) + cl_offset;
	const unsigned int imageX = i % params.width;
	const unsigned int imageY = i / params.width;
	const unsigned int buffIndex = (i - cl_offset);
	
	
	
	int seed = i;
	for(int i=0; i<20; i++)
	{
		rand(&seed);
	}
	
	if(imageY < params.height)
	{
		float2 screenPoint = (float2) {convert_float(imageX), convert_float(imageY)};
		float width = convert_float(params.width);
		float height = convert_float(params.height);
		float resolution = 1.0f/width;
		
		const float3 one = (float3) {1.0f, 0.0f, 0.0f};
		const float3 ones = 1.0f;
		
		matrix33 rot;
		rot.m1 = (float3){1.0f, 0.0f, 0.0f};
		rot.m2 = (float3){0.0f, 1.0f, 0.0f};
		rot.m3 = (float3){0.0f, 0.0f, 1.0f};
		rot = RotateZ(rot, params.alpha);
		rot = RotateX(rot, params.beta);
		rot = RotateY(rot, params.gamma);
		
		float zBuff;
		sStep stepBuff[MAX_RAYMARCHING];
		int buffCount;
		

		//DOF effect
		#if _DOFEnabled
		int blurIndex = 0;
		float3 totalColour = (float3) {0.0, 0.0, 0.0};
		float focus = params.DOFFocus;
		for(blurIndex = 0; blurIndex<256; blurIndex++)
		{
		
			float randR = 0.003 * params.DOFRadius*focus * sqrt(rand(&seed) / 65536.0 / 2.0 + 0.5);
			float randAngle = rand(&seed);
			float randX = randR * sin(randAngle);
			float randZ = randR * cos(randAngle);
			#else
			float randX = 0.0f;
			float randZ = 0.0f;
			float focus = 1.0f;
			#endif //end DOFEnabled
		
			float3 back = (float3) {0.0f + randX/params.zoom, 1.0f, 0.0f + randZ/params.zoom} / params.persp * params.zoom;
			float3 start = params.vp - Matrix33MulFloat3(rot, back);
			//printf("x %f, y %f, start %f %f %f\n", screenPoint.x, screenPoint.y, start.x, start.y, start.z);
			
			float aspectRatio = width / height;
			float x2,z2;
			x2 = (screenPoint.x / width - 0.5) * aspectRatio;
			z2 = (screenPoint.y / height - 0.5);
		
			float x3 = x2 + randX / focus / params.persp;
			float z3 = z2 + randZ / focus / params.persp;

			float3 viewVector = (float3) {x3 * params.persp, 1.0f, z3 * params.persp}; 
			viewVector = Matrix33MulFloat3(rot, viewVector);
			
			bool found = false;
			int count;
			
			float3 point = start;
			float distThresh = 0.0f; 
			float lastDist = 0.01f;
			float depth = 0.1f;
			//ray-marching
			
			float3 startRay = start;
			point = RayMarching(&params, &fractal, startRay, viewVector, 50.0, true, stepBuff, &buffCount, &distThresh,
					&lastDist, &found, &depth);
			
			
			zBuff = depth;
			
			float3 colour = 0.0f;
			if(found)
			{
				float3 normal = NormalVector(&fractal, point, lastDist, distThresh);
				
				float3 lightVector = (float3) {
					cos(params.mainLightAlfa - 0.5 * M_PI) * cos(-params.mainLightBeta), 
					sin(params.mainLightAlfa - 0.5 * M_PI) * cos(-params.mainLightBeta), 
					sin(-params.mainLightBeta)};
				lightVector = Matrix33MulFloat3(rot, lightVector);
				float shade = dot(lightVector, normal);
				if(shade<0.0) shade = 0.0;
				
				float shadow = Shadow(&fractal, point, lightVector, distThresh, resolution);
				//float shadow = 1.0f;
				//shadow = 0.0;
				
				float3 half = lightVector - viewVector;
				half = fast_normalize(half);
				float specular = dot(normal, half);
				if (specular < 0.0f) specular = 0.0f;
				specular = pown(specular, 20);
				if (specular > 15.0) specular = 15.0;
				
				float3 ao = 0.0;
				if (params.fastAmbientOcclusionEnabled) ao = FastAmbientOcclusion(&fractal, point, normal, distThresh, 0.8f, 3);
				if (params.slowAmbientOcclusionEnabled) ao = AmbientOcclusion(&fractal, point, distThresh, params.AmbientOcclusionNoOfVectors, inBuff) * 2.0;
				
				//int colourNumber = outF.colourIndex * params.colouringSpeed + 256.0 * params.colouringOffset;
				int colourNumber = 0;
				float3 surfaceColour = 1.0;
				if (params.colouringEnabled) surfaceColour = IndexToColour(colourNumber, inBuff->palette);
			
				colour = (shade*surfaceColour + specular * params.specularIntensity) * shadow * params.mainLightIntensity + ao * surfaceColour * params.ambientOcclusionIntensity;
			}
			else
			{
				colour = Background(viewVector, &params);
			}
			
			//float3 volFog = VolumetricFog(&params, count-1, distanceBuff, stepBuff);
			//if(volFog.w > 1.0) volFog.w = 1.0;
			//colour = colour * (1.0 - volFog.w) + volFog;
	
			float glow = params.glowIntensity * count / 512.0f;
			float glowN = 1.0f - glow;
			if(glowN < 0.0f) glowN = 0.0f;
			float3 glowColor = params.glowColour1 * glowN + params.glowColour2 * glow;
			colour += glowColor * glow;
		
			#if _DOFEnabled
			totalColour += colour / 256.0;
		}
		
		ushort R = convert_ushort_sat(totalColour.x * 65536.0f);
		ushort G = convert_ushort_sat(totalColour.y * 65536.0f);
		ushort B = convert_ushort_sat(totalColour.z * 65536.0f);
		#else
		ushort R = convert_ushort_sat(colour.x * 65536.0f);
		ushort G = convert_ushort_sat(colour.y * 65536.0f);
		ushort B = convert_ushort_sat(colour.z * 65536.0f);
		#endif //end DOFEnabled
		
		out[buffIndex].R = R;
		out[buffIndex].G = G;
		out[buffIndex].B = B;
		out[buffIndex].zBuffer = zBuff;
	}
}

