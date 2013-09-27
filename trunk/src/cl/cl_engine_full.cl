//#pragma OPENCL EXTENSION cl_amd_fp64 : enable

typedef float3 cl_float3;
typedef float cl_float;
typedef int cl_int;
typedef unsigned int cl_uint;
typedef unsigned short cl_ushort;

#include "mandelbulber_cl_data.h"

#define MAX_RAYMARCHING 10000

typedef struct
{
	float3 z;
	float iters;
	float distance;
	float colourIndex;
} formulaOut;

static formulaOut Fractal(__constant sClInConstants *consts, float3 point, sClCalcParams *calcParam);
static formulaOut CalculateDistance(__constant sClInConstants *consts, float3 point, sClCalcParams *calcParam);

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

float3 Matrix33MulFloat3(matrix33 matrix, float3 vect)
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

float4 quaternionMul(float4 q1, float4 q2)
{
//source: http://www.cs.columbia.edu/~keenan/Projects/QuaternionJulia/QJuliaFragment.html
	float4 result;
	result.x = q1.x * q2.x - dot(q1.yzw, q2.yzw);
	result.yzw = q1.x * q2.yzw + q2.x * q1.yzw + cross(q1.yzw, q2.yzw);
	return result;
}

float4 quaternionSqr(float4 q)
{
	//source: http://www.cs.columbia.edu/~keenan/Projects/QuaternionJulia/QJuliaFragment.html
	float4 result;
	result.x = q.x * q.x - dot(q.yzw, q.yzw);
	result.yzw = 2.0f * q.x * q.yzw;
	return result;
}


/*
float PrimitivePlane(float3 point, float3 centre, float3 normal)
{
	float3 plane = normal;
	plane = plane * (1.0/ length(plane));
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
		float delta = (index2 % 256)/256.0f;
		colOut = col1 + colDiff * delta;
	}
	return colOut;
}

float IterOpacity(float step, float iters, float maxN, float trim, float opacitySp)
{
	float opacity = ((float)iters - trim) / maxN;
	if(opacity < 0.0f) opacity = 0.0f;
	opacity*=opacity;
	opacity*=step  * opacitySp;
	if(opacity > 1.0f) opacity = 1.0f;
	return opacity;
}

float3 CalculateNormals(__constant sClInConstants *consts, sClShaderInputData *input)
{
	float3 normal = 0.0f;
	//calculating normal vector based on distance estimation (gradient of distance function)
	if (!consts->params.slowShading)
	{
		float delta = input->delta;
		//if(input.calcParam->interiorMode) delta = input.dist_thresh * input.param->doubles.quality * 0.2;

		float s1, s2, s3, s4;
		input->calcParam->N *= 5;
		//input.calcParam->minN = 0;

		bool maxIter;

		s1 = CalculateDistance(consts, input->point, input->calcParam).distance;

		float3 deltax = (float3) {delta, 0.0f, 0.0f};
		s2 = CalculateDistance(consts, input->point + deltax, input->calcParam).distance;

		float3 deltay = (float3) {0.0f, delta, 0.0f};
		s3 = CalculateDistance(consts, input->point + deltay, input->calcParam).distance;

		float3 deltaz = (float3) {0.0f, 0.0f, delta};
		s4 = CalculateDistance(consts, input->point + deltaz, input->calcParam).distance;

		normal.x = s2 - s1;
		normal.y = s3 - s1;
		normal.z = s4 - s1;
		input->calcParam->N /= 5;
		//input.calcParam->minN = input.param->fractal.minN;
	}

	//calculating normal vector based on average value of binary central difference
	else
	{
		float result2;
		bool max_iter;

		float3 point2;
		float3 point3;
		for (point2.x = -1.0f; point2.x <= 1.0f; point2.x += 0.2f) //+0.2
		{
			for (point2.y = -1.0f; point2.y <= 1.0f; point2.y += 0.2f)
			{
				for (point2.z = -1.0f; point2.z <= 1.0f; point2.z += 0.2f)
				{
					point3 = input->point + point2 * input->delta;

					float dist = CalculateDistance(consts, point3, input->calcParam).distance;

					if (dist < input->dist_thresh || max_iter) result2 = 0.0f;
					else result2 = 1.0f;

					normal += (point2 * result2);
				}
			}
		}
	}

	if(normal.x == 0.0f && normal.y == 0.0f && normal.z == 0.0f)
	{
		normal.x = 1.0f;
	}
	else
	{
		normal = normalize(normal);
	}

	return normal;
}



float3 RayMarching(__constant sClInConstants *consts, sClCalcParams *calcParam, float3 start, float3 direction, float maxScan, bool binaryEnable, float *distThreshOut, float *lastDistOut, bool *foundOut, float *depthOut, int *stepCount)
{
	float3 point;
	bool found = false;
	float scan = 1e-10f;
	float distThresh = *distThreshOut;
	float dist = 0.0f;
	float search_accuracy = 0.01f * consts->params.quality;
	//printf("consts->params.quality %f, start %f %f %f\n", consts->params.quality, start.x, start.y, start.z);
	float search_limit = 1.0f - search_accuracy;
	float step = 1e-10f;
	float resolution = 1.0f/consts->params.width;
	*stepCount = 0;
	int count = 0;
	float distThreshInit = *distThreshOut;
	//printf("DE_factor %f\n", consts->params.DEfactor);
	
	//printf("Start, start.x = %g, start.y = %g, start.z = %g\n", start.x, start.y, start.z);
	for (int i = 0; i < MAX_RAYMARCHING; i++)
	{
		point = start + direction * scan;
		//printf("viewVector %f %f %f\n", direction.x, direction.y, direction.z);
		//printf("scan %f\n", scan);
		//printf("DE_factor %f\n", consts->params.DEfactor);
		
		bool max_iter = false;

		if (consts->fractal.constantDEThreshold)
		{
			distThresh = consts->params.quality;
			//printf("DistThresh = %f\n", distThresh);
		}
		else
		{
			distThresh = scan * resolution * consts->params.persp / consts->params.quality + distThreshInit;
		}
		//conts->fractal.detailSize = distThresh;
		formulaOut outF;
		outF = CalculateDistance(consts, point, calcParam);
		dist = outF.distance;
		
		//printf("Distance = %f\n", dist/distThresh);

		if (dist > 3.0f) dist = 3.0f;
		if (dist < distThresh)
		{
			found = true;
			break;
		}
		//printf("DE_factor %f\n", consts->params.DEfactor);
		step = (dist - 0.5f * distThresh) * consts->params.DEfactor;
		scan += step;
		if (scan > maxScan)
		{
			break;
		}
		count = i;
	}
	*stepCount = count;
	
	if (found && binaryEnable)
	{
		float step = distThresh * 0.5f;
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
			outF = CalculateDistance(consts, point, calcParam);
					dist = outF.distance;
			//printf("Distance binary = %g\n", dist/distThresh);
			step *= 0.5f;
		}
	}

	*foundOut = found;
	*distThreshOut = distThresh;
	*lastDistOut = dist;
	*depthOut = scan;
	return point;
}

float3 MainShadow(__constant sClInConstants *consts, sClShaderInputData *input)
{
	float3 shadow = 1.0f;

	//starting point
	float3 point2;

	bool max_iter;
	float factor = input->delta / input->resolution;
	if (!consts->params.penetratingLights) factor = consts->params.viewDistanceMax;
	float dist = input->dist_thresh;

	float DE_factor = consts->params.DEfactor;
	if(consts->params.iterFogEnabled || consts->params.volumetricLightEnabledAny) DE_factor = 1.0;

	float start = input->delta;
	//if (input->calcParam->interiorMode) start = input.dist_thresh * DE_factor * 0.5;

	float opacity = 0.0f;
	float shadowTemp = 1.0f;

	float softRange = tan(consts->params.shadowConeAngle / 180.0f * 3.14f);
	float maxSoft = 0.0f;

	bool bSoft = (!consts->params.iterFogEnabled /*&& !input.calcParam->limits_enabled && !input.calcParam->iterThresh*/) && softRange > 0.0;
	
	float distThresh;
	
	for (float i = start; i < factor; i += dist * DE_factor)
	{
		point2 = input->point + input->lightVect * i;

		formulaOut out = CalculateDistance(consts, point2, input->calcParam);
		dist = out.distance;
		
		if(consts->params.iterFogEnabled || consts->params.volumetricLightEnabledAny)
		{
			if (!consts->fractal.constantDEThreshold)
				distThresh = distance(input->eyePoint, point2) * input->resolution * consts->params.persp / consts->params.quality * 0.707106f;
			else
				distThresh = consts->params.quality;
		}
		else
			distThresh = input->dist_thresh;
		
		if (bSoft)
		{
			float angle = (dist - distThresh) / i;
			if (angle < 0.0f) angle = 0.0f;
			if (dist < distThresh) angle = 0.0f;
			float softShadow = (1.0f - angle / softRange);
			if (consts->params.penetratingLights) softShadow *= (factor - i) / factor;
			if (softShadow < 0.0f) softShadow = 0.0f;
			if (softShadow > maxSoft) maxSoft = softShadow;
		}

		if (consts->params.iterFogEnabled)
		{
			opacity = IterOpacity(dist * DE_factor, out.iters, consts->fractal.N, consts->params.iterFogOpacityTrim,
					consts->params.iterFogOpacity);
		}
		else
		{
			opacity = 0.0f;
		}
		shadowTemp -= opacity * (factor - i) / factor;
		
		if (dist < distThresh || shadowTemp < 0.0f)
		{
			shadowTemp -= (factor - i) / factor;
			if (!consts->params.penetratingLights) shadowTemp = 0.0f;
			if (shadowTemp < 0.0f) shadowTemp = 0.0f;
			break;
		}
	}
	if (!bSoft)
	{
		shadow = shadowTemp;
	}
	else
	{
		shadow = (1.0f - maxSoft);
	}
	return shadow;
}

float3 MainSpecular(sClShaderInputData *input)
{
	float3 specular;
	float3 half = input->lightVect - input->viewVector;
	half = normalize(half);
	float shade2 = dot(input->normal,half);
	if (shade2 < 0.0f) shade2 = 0.0f;
	shade2 = pow(shade2, 30.0f) * 1.0f;
	if (shade2 > 15.0f) shade2 = 15.0f;
	specular = shade2;
	return specular;
}

float3 SurfaceColour(__constant sClInConstants *consts, sClShaderInputData *input, global cl_float3 *palette)
{
	formulaOut outF = Fractal(consts, input->point, input->calcParam);
	int colourNumber = outF.colourIndex * consts->params.colouringSpeed + 256.0 * consts->params.colouringOffset;
	float3 surfaceColour = 1.0;
	if (consts->params.colouringEnabled) surfaceColour = IndexToColour(colourNumber, palette);
	return surfaceColour;
}

float3 FastAmbientOcclusion2(__constant sClInConstants *consts, sClShaderInputData *input)
{
	//reference: http://www.iquilezles.org/www/material/nvscene2008/rwwtt.pdf (Iñigo Quilez – iq/rgba)

	float delta = input->dist_thresh;
	float aoTemp = 0.0f;
	int quality = consts->params.globalIlumQuality;
	for (int i = 1; i < quality * quality; i++)
	{
		float scan = i * i * delta;
		float3 pointTemp = input->point + input->normal * scan;
		float 		dist = CalculateDistance(consts, pointTemp, input->calcParam).distance;
		aoTemp += 1.0f / (pow(2.0f, i)) * (scan - consts->params.fastAoTune * dist) / input->dist_thresh;
	}
	float ao = 1.0f - 0.2f * aoTemp;
	if (ao < 0.0f) ao = 0.0f;
	float3 output = ao;
	return output;
}

#if _SlowAOEnabled
float3 AmbientOcclusion(__constant sClInConstants *consts, sClShaderInputData *input, global float3 *vectorsAround, global float3 *vectorsAroundColours)
{
	float3 AO = 0.0f;

	float start_dist = input->delta;
	float end_dist = input->delta / input->resolution;
	float intense = 0.0f;
	float distThresh;
	
	for (int i = 0; i < input->vectorsCount; i++)
	{
		float3 v = vectorsAround[i];
		float3 colour = vectorsAroundColours[i];

		//printf("ao vector %d, %f %f %f\n", i, v.x, v.y, v.z);
		
		float dist = input->lastDist;

		float opacity = 0.0;
		float shadowTemp = 1.0;

		int count = 0;
		for (float r = start_dist; r < end_dist; r += dist * 2.0f)
		{
			float3 point2 = input->point + v * r;

			formulaOut out = CalculateDistance(consts, point2, input->calcParam);
			dist = out.distance;

			if (consts->params.iterFogEnabled)
			{
				opacity = IterOpacity(dist * 2.0f, out.iters, consts->fractal.N, consts->params.iterFogOpacityTrim,
						consts->params.iterFogOpacity);
			}
			else
			{
				opacity = 0.0;
			}
			shadowTemp -= opacity * (end_dist - r) / end_dist;

			if(consts->params.iterFogEnabled || consts->params.volumetricLightEnabledAny)
			{
				if (!consts->fractal.constantDEThreshold)
					distThresh = distance(input->eyePoint, point2) * input->resolution * consts->params.persp / consts->params.quality * 0.707106f;
				else
					distThresh = consts->params.quality;
			}
			else
				distThresh = input->dist_thresh;
			
			if (dist < distThresh || shadowTemp < 0.0)
			{
				shadowTemp -= (end_dist - r) / end_dist;
				if (shadowTemp < 0.0) shadowTemp = 0.0;
				break;
			}
			if (count > 100) break;
		}

		intense = shadowTemp;

		AO += intense * colour;

	}
	AO = AO / input->vectorsCount;

	return AO;
}
#endif

#if _AuxLightsEnabled
float AuxShadow(__constant sClInConstants *consts, sClShaderInputData *input, float distanceToLight, float3 lightVector)
{
	
	float step = input->delta;
	float dist = step;
	float light = 1.0f;

	float opacity = 0.0f;
	float shadowTemp = 1.0f;

	//float DE_factor = consts->params.DEfactor;
	float DE_factor = 1.0;
	
	float distThresh;

	if(consts->params.iterFogEnabled || consts->params.volumetricLightEnabled[0]) DE_factor = 1.0f;
	int count = 0;
	//printf("start, thresh = %f, distance = %f\n", input->dist_thresh, distanceToLight);
	for (float i = input->delta; i < distanceToLight; i += dist * DE_factor)
	{
		//printf("i = %f, dist = %f\n", i, dist);
		float3 point2 = input->point + lightVector * i;

		formulaOut out = CalculateDistance(consts, point2, input->calcParam);
		dist = out.distance;
		
		if (consts->params.iterFogEnabled)
		{
			opacity = IterOpacity(dist * DE_factor, out.iters, consts->fractal.N, consts->params.iterFogOpacityTrim,
					consts->params.iterFogOpacity);
		}
		else
		{
			opacity = 0.0f;
		}
		shadowTemp -= opacity * (distanceToLight - i) / distanceToLight;

		if(consts->params.iterFogEnabled || consts->params.volumetricLightEnabledAny)
		{
			if (!consts->fractal.constantDEThreshold)
				distThresh = distance(input->eyePoint, point2) * input->resolution * consts->params.persp / consts->params.quality * 0.707106f;
			else
				distThresh = consts->params.quality;
		}
		else
			distThresh = input->dist_thresh;
		
		if (dist < distThresh || shadowTemp < 0.0f || count > 1000)
		{
			if (consts->params.penetratingLights)
			{
				shadowTemp -= (distanceToLight - i) / distanceToLight;
				if (shadowTemp < 0.0f) shadowTemp = 0.0f;
			}
			else
			{
				shadowTemp = 0.0f;
			}
			break;
		}
		count ++;
	}
	light = shadowTemp;
	return light;
}

float3 LightShading(__constant sClInConstants *consts, sClShaderInputData *input, sClLight light, int number, float3 *specularOut)
{
	float3 shading;

	float3 d = light.position - input->point;

	float distance = length(d);

	//angle of incidence
	float3 lightVector = d;
	lightVector = normalize(lightVector);

	float intensity = consts->params.auxLightIntensity * 100.0f * light.intensity / (distance * distance) / number;
	float shade = dot(input->normal, lightVector);
	if (shade < 0.0f) shade = 0.0f;
	shade = shade * intensity;
	if (shade > 500.0f) shade = 500.0f;

	//specular
	float3 half = lightVector - input->viewVector;
	half = normalize(half);
	float shade2 = dot(input->normal, half);
	if (shade2 < 0.0f) shade2 = 0.0f;
	shade2 = pow(shade2, 30.0f) * 1.0f;
	shade2 *= intensity * consts->params.specularIntensity;
	if (shade2 > 15.0) shade2 = 15.0;

	//calculate shadow
	if ((shade > 0.01f || shade2 > 0.01f) && consts->params.shadow)
	{
		float light = AuxShadow(consts, input, distance, lightVector);
		shade *= light;
		shade2 *= light;
	}
	else
	{
		if (consts->params.shadow)
		{
			shade = 0.0f;
			shade2 = 0.0f;
		}
	}

	shading = shade * light.colour;
	(*specularOut) = shade2 * light.colour;

	return shading;
}

float3 AuxLightsShader(__constant sClInConstants *consts, sClShaderInputData *input, float3 *specularOut, global sClLight *lights)
{
	int numberOfLights = consts->params.auxLightNumber;

	float3 shadeAuxSum = 0.0f;
	float3 specularAuxSum = 0.0f;
	for (int i = 0; i < numberOfLights; i++)
	{
		if (lights[i].enabled)
		{
			float3 specularAuxOutTemp;
			float3 shadeAux = LightShading(consts, input, lights[i], numberOfLights, &specularAuxOutTemp);
			shadeAuxSum += shadeAux;
			specularAuxSum += specularAuxOutTemp;
		}
	}
	*specularOut = specularAuxSum;
	return shadeAuxSum;
}
#endif

float3 ObjectShader(__constant sClInConstants *consts, sClShaderInputData *input, float3 *specularOut, global sClInBuff *inBuff)
{
	float3 output;

	//normal vector
	float3 vn = CalculateNormals(consts, input);
	input->normal = vn;
	
	float3 mainLight = consts->params.mainLightIntensity * consts->params.mainLightColour;
	
	//calculate shading based on angle of incidence
	float shadeTemp = dot(input->normal, input->lightVect);
	if (shadeTemp < 0.0f) shadeTemp = 0.0f;
	shadeTemp = consts->params.mainLightIntensity * ((1.0f - consts->params.shading) + consts->params.shading * shadeTemp);
	float3 shade = shadeTemp;
	
	//calculate shadow
	float3 shadow = 1.0f;
	if(consts->params.shadow) shadow = MainShadow(consts, input);
	
	//calculate specular highlight
	float3 specular = MainSpecular(input) * consts->params.specularIntensity;
	
	//calculate surface colour
	float3 colour = SurfaceColour(consts, input, inBuff->palette);
	
	//ambient occlusion
	float3 ambient = 0.0f;
	if(consts->params.fastAmbientOcclusionEnabled)
	{
		ambient = FastAmbientOcclusion2(consts, input);
	}
	else if(consts->params.slowAmbientOcclusionEnabled)
	{
#if _SlowAOEnabled
		ambient = AmbientOcclusion(consts, input, inBuff->vectorsAround, inBuff->vectorsAroundColours);
#endif
	}
	float3 ambient2 = ambient * consts->params.ambientOcclusionIntensity;
	
	//additional lights
	float3 auxLights = 0.0f;
	float3 auxLightsSpecular = 0.0f;
#if _AuxLightsEnabled
	auxLights = AuxLightsShader(consts, input, &auxLightsSpecular, inBuff->lights);
	auxLightsSpecular *= consts->params.specularIntensity;
#endif
	
	output = mainLight * shadow * (shade * colour + specular) + ambient2 * colour + auxLights * colour + auxLightsSpecular;
	
	return output;
}

float3 BackgroundShader(__constant sClInConstants *consts, sClShaderInputData *input)
{
	float3 vector = { 0.0f, 0.0f, -1.0f };
	vector = normalize(vector);
	float grad = dot(input->viewVector, vector) + 1.0f;
	float3 colour;
	if (grad < 1.0f)
	{
		float ngrad = 1.0f - grad;
		colour = consts->params.backgroundColour3 * ngrad + consts->params.backgroundColour2 * grad;
	}
	else
	{
		grad = grad - 1.0f;
		float ngrad = 1.0f - grad;
		colour = consts->params.backgroundColour2 * ngrad + consts->params.backgroundColour1 * grad;
	}
	return colour;
}

float3 VolumetricShader(__constant sClInConstants *consts, sClShaderInputData *input, global sClInBuff *inBuff, float3 oldPixel)
{
	float3 output = oldPixel;
	float scan = input->depth;
	if(scan > consts->params.viewDistanceMax) scan = consts->params.viewDistanceMax;
	float dist = input->lastDist;
	float distThresh = input->dist_thresh;
	float delta = input->delta;
	bool end = false;
	
	//glow init
  float glow = input->stepCount * consts->params.glowIntensity / 512.0f * consts->params.DEfactor;
  float glowN = 1.0f - glow;
  if (glowN < 0.0f) glowN = 0.0f;
  float3 glowColour = (consts->params.glowColour1 * glowN + consts->params.glowColour2 * glow);
	
  //volumetric fog init
	float colourThresh = consts->params.fogColour1Distance;
	float colourThresh2 = consts->params.fogColour2Distance;
	float fogReduce = consts->params.fogDistanceFactor;
	float fogIntensity = consts->params.fogDensity;
  
	for(int i=0; i<MAX_RAYMARCHING; i++)
	{
		//calculate back step
		float step = dist * consts->params.DEfactor * (0.9f + 0.2f * rand(&input->calcParam->randomSeed)/65536.0);

		if(step < delta) step = delta;
		if(step > scan) 
		{
			step = scan;
			end = true;
		}
		//else if(step > 100.0f * distThresh) step = 100.0f * distThresh;
		scan -= step;
		
		float3 point = input->startPoint + input->viewVector * scan;
		input->point = point;
		
		formulaOut out = CalculateDistance(consts, point, input->calcParam);
		dist = out.distance;
		
		if (consts->fractal.constantDEThreshold)
		{
			distThresh = consts->params.quality;
		}
		else
		{
			distThresh = distance(point, input->eyePoint) * input->resolution * consts->params.persp / consts->params.quality;
		}
		input->dist_thresh = distThresh;
		
		if(consts->fractal.constantDEThreshold) input->delta = scan * input->resolution * consts->params.persp;
		else input->delta = distThresh * consts->params.quality;
		delta = input->delta;
		
		
		
		//------------------- glow
		float glowOpacity = glow * step / input->depth;
		output = glowOpacity * glowColour + (1.0f - glowOpacity) * output;
	
		
		//------------------ visible light
		if (consts->params.auxLightVisibility > 0.0f)
		{
			float lowestLightSize = 1.0e10f;
			float lowestLightDist = 1.0e10f;
			for (int i = 0; i < consts->params.auxLightNumber; ++i)
			{
				if (inBuff->lights[i].enabled)
				{
					float3 lightDistVect = point - inBuff->lights[i].position;
					float lightDist = length(lightDistVect);
					float lightSize = inBuff->lights[i].intensity * consts->params.auxLightIntensity * consts->params.auxLightVisibility;
					float r2 = lightDist / lightSize;
					if (r2 < lowestLightSize)
					{
							lowestLightSize = r2;
							lowestLightDist = lightDist;
					}
				}
			}

			//small steps close to light source to improve accuracy
			int smallSteps = 0;
			int smallStep_start = 0;
			int smallStep_end = 1;
			float step2 = step;

			smallSteps = 10.0f * step / (lowestLightDist + 1.0e-15f);
			if(smallSteps > 50) smallSteps = 50;
			if (smallSteps > 0)
			{
				smallStep_start = 1;
				smallStep_end = smallSteps + 1;
				step2 = step / (smallSteps + 1.0f);
			}

			for (int smallStep = smallStep_start; smallStep < smallStep_end; smallStep++)
			{
				float3 point2 = point + input->viewVector * step2 * smallStep;

				for (int i = 0; i < consts->params.auxLightNumber; ++i)
				{
					if (inBuff->lights[i].enabled)
					{
						float3 lightDistVect = point2 - inBuff->lights[i].position;
						float lightDist = length(lightDistVect);
						float lightSize = inBuff->lights[i].intensity * consts->params.auxLightIntensity * consts->params.auxLightVisibility;
						float r2 = lightDist / lightSize;
						if (r2 > 1.0f) r2 = 1.0f;
						float bellFunction = (cos(r2 * M_PI_F) + 1.0f) / (r2 * r2 + 0.02f) * 0.3f;
						float lightDensity = step2 * bellFunction / lightSize;

						output += lightDensity * inBuff->lights[i].colour;
					}
				}
			}
		}
		
		
		//---------------------- volumetric lights with shadows in fog


		for (int i = 0; i < 5; i++)
		{
			if (i == 0 && consts->params.volumetricLightEnabled[0])
			{
				float3 shadowOutputTemp = MainShadow(consts, input);
				output += shadowOutputTemp * step * consts->params.volumetricLightIntensity[0] * consts->params.mainLightColour;
			}
#if _AuxLightsEnabled
			if (i > 0)
			{
				if (inBuff->lights[i - 1].enabled && consts->params.volumetricLightEnabled[i])
				{
					float3 d = inBuff->lights[i - 1].position - point;
					float distance = length(d);
					if(distance > 0.0f)
					{
						float distance2 = distance * distance;
						float3 lightVectorTemp = d;
						lightVectorTemp = normalize(lightVectorTemp);
						float light = AuxShadow(consts, input, distance, lightVectorTemp);
						//float light = 1.0;
						output += light * inBuff->lights[i - 1].colour * consts->params.volumetricLightIntensity[i] * step / distance2;
					}
				}
			}
#endif
		}

		
		//----------------------- basic fog
		if(consts->params.fogEnabled)
		{
			float fogDensity = step / consts->params.fogVisibility;
			if(fogDensity > 1.0f) fogDensity = 1.0f;
			output = fogDensity * consts->params.fogColour + (1.0f - fogDensity) * output;
		}
		
		
		//-------------------- volumetric fog
		if(fogIntensity > 0.0f)
		{
			float densityTemp = (step * fogReduce) / (dist * dist + fogReduce * fogReduce);

			float k = dist / colourThresh;
			if (k > 1.0f) k = 1.0f;
			float kn = 1.0f - k;
			float3 fogTemp = (consts->params.fogColour1 * kn + consts->params.fogColour2 * k);

			float k2 = dist / colourThresh2 * k;
			if (k2 > 1.0f) k2 = 1.0f;
			kn = 1.0f - k2;
			fogTemp = fogTemp * kn + consts->params.fogColour3 * k2;

			float fogDensity = 0.3f * fogIntensity * densityTemp / (1.0f + fogIntensity * densityTemp);
			if(fogDensity > 1.0f) fogDensity = 1.0f;

			output = fogDensity * fogTemp + (1.0 - fogDensity) * output;
		}
		
		//iter fog
		
		if (consts->params.iterFogEnabled)
		{
			int L = out.iters;
			float opacity = IterOpacity(step, L, consts->fractal.N, consts->params.iterFogOpacityTrim, consts->params.iterFogOpacity);

			float3 newColour = 0.0f;
			if (opacity > 0)
			{
				//fog colour
				float iterFactor = (float) 2.0f * (L - consts->params.iterFogOpacityTrim) / (consts->fractal.N - consts->params.iterFogOpacityTrim);
				float k = iterFactor;
				if (k > 1.0) k = 1.0;
				if (k < 0.0) k = 0.0;
				float kn = 1.0 - k;
				float3 fogCol = (consts->params.fogColour1 * kn + consts->params.fogColour2 * k);

				float k2 = iterFactor - 1.0f;
				if (k2 < 0.0f) k2 = 0.0f;
				if (k2 > 1.0f) k2 = 1.0f;
				kn = 1.0f - k2;
				fogCol = (fogCol * kn + consts->params.fogColour3 * k2);

				//----

				
				for (int i = 0; i < 5; i++)
				{
					if (i == 0)
					{
						if (consts->params.mainLightIntensity > 0.0)
						{
							float3 shadowOutputTemp = MainShadow(consts, input);
							newColour += shadowOutputTemp * consts->params.mainLightColour * consts->params.mainLightIntensity;
						}
					}

#if _AuxLightsEnabled
					if (i > 0)
					{
						if (inBuff->lights[i - 1].enabled)
						{
							float3 d = inBuff->lights[i - 1].position - point;
							float distance = length(d);
							float distance2 = distance * distance;
							float3 lightVectorTemp = normalize(d);
							float3 light = AuxShadow(consts, input, distance, lightVectorTemp);
							float3 intensity = inBuff->lights[i - 1].intensity * 100.0f;
							newColour += light * inBuff->lights[i - 1].colour / distance2 * intensity;
						}
					}
#endif
				}

				
				//if (input.param->global_ilumination && !input.param->fastGlobalIllumination)
				//{
				//	sShaderOutput AO = AmbientOcclusion(input);
				//	newColour.R += AO.R * input.param->doubles.imageAdjustments.globalIlum;
				//	newColour.G += AO.G * input.param->doubles.imageAdjustments.globalIlum;
				//	newColour.B += AO.B * input.param->doubles.imageAdjustments.globalIlum;
				//}
				
				if (opacity > 1.0f) opacity = 1.0f;

				output = output * (1.0f - opacity) + newColour * opacity * fogCol;
			}
		}
		
		
		if(end) break;
	}

	return output;
}

//------------------ MAIN RENDER FUNCTION --------------------
kernel void fractal3D(__global sClPixel *out, __global sClInBuff *inBuff, __constant sClInConstants *consts, __global sClReflect *reflectBuff, int Gcl_offset)
{
	
	int cl_offset = Gcl_offset;
	const unsigned int i = get_global_id(0) + cl_offset;
	const unsigned int imageX = i % consts->params.width;
	const unsigned int imageY = i / consts->params.width;
	const unsigned int buffIndex = (i - cl_offset);
	
	const size_t local_id = get_local_id(0);
	const size_t local_size = get_local_size(0);
	const size_t group_id = get_group_id(0);
	const int local_offset = get_global_id(0) * 10;
	
	
	int seed = i + consts->params.randomSeed;
	for(int i=0; i<20; i++)
	{
		rand(&seed);
	}
	
	if(imageY < consts->params.height)
	{
		float2 screenPoint = (float2) {convert_float(imageX), convert_float(imageY)};
		float width = convert_float(consts->params.width);
		float height = convert_float(consts->params.height);
		float resolution = 1.0f/width;
		
		const float3 one = (float3) {1.0f, 0.0f, 0.0f};
		const float3 ones = 1.0f;
		
		matrix33 rot;
		rot.m1 = (float3){1.0f, 0.0f, 0.0f};
		rot.m2 = (float3){0.0f, 1.0f, 0.0f};
		rot.m3 = (float3){0.0f, 0.0f, 1.0f};
		rot = RotateZ(rot, consts->params.alpha);
		rot = RotateX(rot, consts->params.beta);
		rot = RotateY(rot, consts->params.gamma);
		
		float zBuff;
		//local sClStep stepBuff[MAX_RAYMARCHING];
		int buffCount;
		
		int maxRay = consts->params.reflectionsMax;		

		sClCalcParams calcParam;
		calcParam.N = consts->fractal.N;

		float3 colour = 0.0f;
		float3 resultShader = 0.0f;
		
		
		//DOF effect
		#if _DOFEnabled
		int blurIndex = 0;
		float3 totalColour = (float3) {0.0f, 0.0f, 0.0f};
		float focus = consts->params.DOFFocus;

		float randR = 0.003f * consts->params.DOFRadius*focus * sqrt(rand(&seed) / 65536.0 / 2.0f + 0.5f);
		float randAngle = rand(&seed);
		float randX = randR * sin(randAngle);
		float randZ = randR * cos(randAngle);
#else
		float randX = 0.0f;
		float randZ = 0.0f;
		float focus = 1.0f;
#endif //end DOFEnabled
		float3 start = 0.0f;
		if (consts->params.perspectiveType == 1 || consts->params.perspectiveType == 2)
		{
			float3 back = (float3)
			{ randX, 0.0f, randZ};
			start = consts->params.vp - Matrix33MulFloat3(rot, back);
		}
		else
		{
			float3 back = (float3) {randX/consts->params.zoom, 1.0f, randZ/consts->params.zoom } / consts->params.persp * consts->params.zoom;
			start = consts->params.vp - Matrix33MulFloat3(rot, back);
		}
		//printf("x %f, y %f, start %f %f %f\n", screenPoint.x, screenPoint.y, start.x, start.y, start.z);

		float aspectRatio = width / height;
		float x2,z2;
		x2 = (screenPoint.x / width - 0.5f) * aspectRatio;
		z2 = (screenPoint.y / height - 0.5f);

		float x3, z3;
		if (consts->params.perspectiveType == 1 || consts->params.perspectiveType == 2)
		{
			x3 = x2 + randX / focus / M_PI_F / consts->params.persp;
			z3 = z2 + randZ / focus / M_PI_F / consts->params.persp;
		}
		else
		{
			x3 = x2 + randX / focus / consts->params.persp / consts->params.persp;
			z3 = z2 + randZ / focus / consts->params.persp / consts->params.persp;
		}
		
		float3 viewVector;
		bool hemisphereCut = false;
		float fov = consts->params.persp;

		if (consts->params.perspectiveType == 1)//fish eye
		{
			if (consts->params.fishEyeCut && length((float2)
							{ x2, z2}) > 0.5f / fov)
			{
				hemisphereCut = true;
			}

			float x4 = x3 * M_PI_F;
			float z4 = z3 * M_PI_F;
			float r = length((float2)
					{ x4, z4});

			if(r == 0.0f)
			{
				viewVector.x = 0.0f;
				viewVector.z = 0.0f;
				viewVector.y = 1.0f;
			}
			else
			{
				viewVector.x = x4 / r * sin(r * fov);
				viewVector.z = z4 / r * sin(r * fov);
				viewVector.y = cos(r * fov);
			}
		}
		else if(consts->params.perspectiveType == 2) //equirectangular
		{
			float x4 = x3 * M_PI_F;
			float z4 = z3 * M_PI_F;

			viewVector.x = sin(fov * x4) * cos(fov * z4);
			viewVector.z = sin(fov * z4);
			viewVector.y = cos(fov * x4) * cos(fov * z4);
		}
		else //three-point perspective
		{
			viewVector = (float3)
			{ x3 * fov, 1.0f, z3 * fov};
		}

		//rotate vieviector by view angle
		viewVector = Matrix33MulFloat3(rot, viewVector);

		//ray-marching			
		float distThresh = 0.0f;
		float3 point = start;
		float3 startRay = start;
		float lastDist = 0.0f;
		bool found = false;
		float depth = 0.0f;
		int stepCount = 0;

		float3 objectColour = 0.0f;

		float3 lightVector = (float3)
		{
			cos(consts->params.mainLightAlfa - 0.5f * M_PI_F) * cos(-consts->params.mainLightBeta),
			sin(consts->params.mainLightAlfa - 0.5f * M_PI_F) * cos(-consts->params.mainLightBeta),
			sin(-consts->params.mainLightBeta)};
		lightVector = Matrix33MulFloat3(rot, lightVector);

		int rayEnd = 0;

		//printf("start ray-tracing\n");
		//printf("Max ray = %d\n", maxRay);
		for (int ray = 0; ray <= maxRay; ray++)
		{
			sClShaderInputData shaderInputData;
			reflectBuff[ray + local_offset].start = startRay;
			reflectBuff[ray + local_offset].viewVector = viewVector;
			reflectBuff[ray + local_offset].found = false;
			//printf("startRay %f %f %f\n", startRay.x, startRay.y, startRay.z);
			//printf("viewVector %f %f %f\n", viewVector.x, viewVector.y, viewVector.z);
			//reflectBuff[ray].buffCount = 0;

			point = RayMarching(consts, &calcParam, startRay, viewVector, consts->params.viewDistanceMax, true, &distThresh, &lastDist, &found, &depth, &stepCount);

			//printf("point %f %f %f\n", point.x, point.y, point.z);

			reflectBuff[ray + local_offset].depth = depth;
			reflectBuff[ray + local_offset].found = found;
			reflectBuff[ray + local_offset].lastDist = lastDist;
			reflectBuff[ray + local_offset].point = point;
			reflectBuff[ray + local_offset].distThresh = distThresh;
			reflectBuff[ray + local_offset].stepCount = stepCount;

			//printf("reflectBuff[ray].distThresh %f\n", reflectBuff[ray + local_offset].distThresh);

			//reflectBuff[ray].objectType = calcParam.objectOut;

			rayEnd = ray;
			if(!found) break;
			//if(reflectBuff[ray].reflect == 0) break;

			//calculate new ray direction and start point
			startRay = point;

			shaderInputData.calcParam = &calcParam;
			shaderInputData.dist_thresh = distThresh;
			shaderInputData.lightVect = lightVector;
			shaderInputData.point = point;
			shaderInputData.viewVector = viewVector;
			if(consts->fractal.constantDEThreshold) shaderInputData.delta = depth * resolution * consts->params.persp;
			else shaderInputData.delta = distThresh * consts->params.quality;
			if(shaderInputData.delta < 1.0e-6) shaderInputData.delta = 1.0e-6;
			float3 vn = CalculateNormals(consts, &shaderInputData);
			viewVector = viewVector - vn * dot(viewVector,vn) * 2.0f;
			startRay = startRay + viewVector * distThresh;
		}

		for(int ray = rayEnd; ray >= 0; ray--)
		{
			sClShaderInputData shaderInputData;
			float3 objectShader = 0.0f;
			float3 backgroudShader = 0.0f;
			float3 volumetricShader = 0.0f;
			float3 specular = 0.0f;

			calcParam.randomSeed = seed;
			shaderInputData.calcParam = &calcParam;

			shaderInputData.dist_thresh = reflectBuff[ray + local_offset].distThresh;

			if(consts->fractal.constantDEThreshold) shaderInputData.delta = reflectBuff[ray + local_offset].depth * resolution * consts->params.persp;
			else shaderInputData.delta = reflectBuff[ray + local_offset].distThresh * consts->params.quality;
			if(shaderInputData.delta < 1.0e-6) shaderInputData.delta = 1.0e-6;
			
			shaderInputData.lightVect = lightVector;
			shaderInputData.point = reflectBuff[ray + local_offset].point;
			shaderInputData.startPoint = reflectBuff[ray + local_offset].start;
			shaderInputData.eyePoint = start;
			shaderInputData.viewVector = reflectBuff[ray + local_offset].viewVector;
			shaderInputData.vectorsCount = consts->params.AmbientOcclusionNoOfVectors;
			shaderInputData.lastDist = reflectBuff[ray + local_offset].lastDist;
			shaderInputData.depth = reflectBuff[ray + local_offset].depth;
			shaderInputData.stepCount = reflectBuff[ray + local_offset].stepCount;
			shaderInputData.resolution = resolution;
			//shaderInputData.envMappingTexture = param.envmapTexture;
			//shaderInputData.objectType = reflectBuff[ray].objectType;
			//shaderInputData.calcParam->doubles.detailSize = reflectBuff[ray].distThresh;

			if(reflectBuff[ray + local_offset].found)
			{
				//printf("Last dist %f = \n", lastDist / distThresh);	
				objectShader = ObjectShader(consts, &shaderInputData, &specular, inBuff);
			}
			else
			{
				backgroudShader = BackgroundShader(consts, &shaderInputData);
				reflectBuff[ray + local_offset].depth = 1e20f;
			}

			if (maxRay > 0 && rayEnd > 0 && ray != rayEnd)
			{
				colour = resultShader * consts->params.reflect;
				colour += (1.0f - consts->params.reflect) * (objectShader + backgroudShader) + specular;
			}
			else
			{
				colour = objectShader + backgroudShader + specular;
			}
			resultShader = VolumetricShader(consts, &shaderInputData, inBuff, colour);
			//resultShader = colour;

			//seed = shaderInputData.calcParam->randomSeed;

		}
		zBuff = reflectBuff[0 + local_offset].depth;

		float3 finallColour = resultShader;
		
		out[buffIndex].R = finallColour.x;
		out[buffIndex].G = finallColour.y;
		out[buffIndex].B = finallColour.z;
		out[buffIndex].zBuffer = zBuff;
		
	}
}

