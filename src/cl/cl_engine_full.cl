//#pragma OPENCL EXTENSION cl_amd_fp64 : enable

typedef float3 cl_float3;
typedef float cl_float;
typedef int cl_int;
typedef unsigned int cl_uint;
typedef unsigned short cl_ushort;

#include "cl_data.h"

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
		float delta = (index2 % 256)/256.0f;
		colOut = col1 + colDiff * delta;
	}
	return colOut;
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
		normal = fast_normalize(normal);
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
	//if(consts->params.imageSwitches.iterFogEnabled || input.param->imageSwitches.volumetricLightEnabled) DE_factor = 1.0;

	float start = input->delta;
	//if (input->calcParam->interiorMode) start = input.dist_thresh * DE_factor * 0.5;

	float opacity = 0.0f;
	float shadowTemp = 1.0f;

	float softRange = tan(consts->params.shadowConeAngle / 180.0f * 3.14f);
	float maxSoft = 0.0f;

	//bool bSoft = (!consts->params.imageSwitches.iterFogEnabled && !input.calcParam->limits_enabled && !input.calcParam->iterThresh) && softRange > 0.0;
	bool bSoft = softRange > 0.0f;
	
	for (float i = start; i < factor; i += dist * DE_factor)
	{
		point2 = input->point + input->lightVect * i;

		dist = CalculateDistance(consts, point2, input->calcParam).distance;

		if (bSoft)
		{
			float angle = (dist - input->dist_thresh) / i;
			if (angle < 0.0f) angle = 0.0f;
			if (dist < input->dist_thresh) angle = 0.0f;
			float softShadow = (1.0f - angle / softRange);
			if (consts->params.penetratingLights) softShadow *= (factor - i) / factor;
			if (softShadow < 0.0f) softShadow = 0.0f;
			if (softShadow > maxSoft) maxSoft = softShadow;
		}

		//if (consts->params.imageSwitches.iterFogEnabled)
		//{
		//	opacity = IterOpacity(dist * DE_factor, input->calcParam->itersOut, input->calcParam->doubles.N, consts->params.doubles.iterFogOpacityTrim,
		//			input.param->doubles.iterFogOpacity);
		//}
		//else
		//{
		//	opacity = 0.0f;
		//}
		//shadowTemp -= opacity * (factor - i) / factor;

		if (dist < input->dist_thresh || shadowTemp < 0.0f)
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
	half = fast_normalize(half);
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

float3 AmbientOcclusion(__constant sClInConstants *consts, sClShaderInputData *input, global float3 *vectorsAround, global float3 *vectorsAroundColours)
{
	float3 AO = 0.0f;

	float start_dist = input->delta;
	float end_dist = input->delta / input->resolution;
	float intense = 0.0f;

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

			dist = CalculateDistance(consts, point2, input->calcParam).distance;

			//if (consts->params.imageSwitches.iterFogEnabled)
			//{
			//	opacity = IterOpacity(dist * input.param->doubles.DE_factor, input.calcParam->itersOut, input.calcParam->doubles.N, input.param->doubles.iterFogOpacityTrim,
			//			input.param->doubles.iterFogOpacity);
			//}
			//else
			//{
			//	opacity = 0.0;
			//}
			//shadowTemp -= opacity * (end_dist - r) / end_dist;

			if (dist < input->dist_thresh || shadowTemp < 0.0)
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

float AuxShadow(__constant sClInConstants *consts, sClShaderInputData *input, float distance, float3 lightVector)
{
	float step = input->delta;
	float dist = step;
	float light = 1.0f;

	float opacity = 0.0f;
	float shadowTemp = 1.0f;

	float DE_factor = consts->params.DEfactor;
	//if(consts->params.imageSwitches.iterFogEnabled || consts->params.imageSwitches.volumetricLightEnabled) DE_factor = 1.0;

	for (float i = input->delta; i < distance; i += dist * DE_factor)
	{
		float3 point2 = input->point + lightVector * i;

		dist = CalculateDistance(consts, point2, input->calcParam).distance;

		//if (consts->params.imageSwitches.iterFogEnabled)
		//{
		//	opacity = IterOpacity(dist * DE_factor, input.calcParam->itersOut, input.calcParam->doubles.N, input.param->doubles.iterFogOpacityTrim,
		//			input.param->doubles.iterFogOpacity);
		//}
		//else
		//{
		//	opacity = 0.0;
		//}
		//shadowTemp -= opacity * (distance - i) / distance;

		if (dist < input->dist_thresh || shadowTemp < 0.0f)
		{
			if (consts->params.penetratingLights)
			{
				shadowTemp -= (distance - i) / distance;
				if (shadowTemp < 0.0f) shadowTemp = 0.0f;
			}
			else
			{
				shadowTemp = 0.0f;
			}
			break;
		}
	}
	light = shadowTemp;
	return light;
}

float3 LightShading(__constant sClInConstants *consts, sClShaderInputData *input, sClLight light, int number, float3 *specularOut)
{
	float3 shading;

	float3 d = light.position - input->point;

	float distance = fast_length(d);

	//angle of incidence
	float3 lightVector = d;
	lightVector = fast_normalize(lightVector);

	float intensity = consts->params.auxLightIntensity * 100.0f * light.intensity / (distance * distance) / number;
	float shade = dot(input->normal, lightVector);
	if (shade < 0.0f) shade = 0.0f;
	shade = shade * intensity;
	if (shade > 500.0f) shade = 500.0f;

	//specular
	float3 half = lightVector - input->viewVector;
	half = fast_normalize(half);
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
	float3 specular = MainSpecular(input);
	
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
		ambient = AmbientOcclusion(consts, input, inBuff->vectorsAround, inBuff->vectorsAroundColours);
	}
	float3 ambient2 = ambient * consts->params.ambientOcclusionIntensity;
	
	//additional lights
	float3 auxLights;
	float3 auxLightsSpecular;
	auxLights = AuxLightsShader(consts, input, &auxLightsSpecular, inBuff->lights);
	
	output = mainLight * shadow * (shade * colour + specular) + ambient2 * colour + auxLights * colour + auxLightsSpecular;
	
	return output;
}

float3 BackgroundShader(__constant sClInConstants *consts, sClShaderInputData *input)
{
	float3 vector = { 1.0f, 1.0f, -1.0f };
	vector = fast_normalize(vector);
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

float3 VolumetricShader(__constant sClInConstants *consts, sClShaderInputData *input, float3 oldPixel)
{
	float3 output = oldPixel;
	float scan = input->depth;
	float dist = input->lastDist;
	float distThresh = input->dist_thresh;
	bool end = false;
	
	//glow init
  float glow = input->stepCount * consts->params.glowIntensity / 512.0f * consts->params.DEfactor;
  float glowN = 1.0f - glow;
  if (glowN < 0.0f) glowN = 0.0f;
  float3 glowColour = (consts->params.glowColour1 * glowN + consts->params.glowColour2 * glow);
	
	for(int i=0; i<MAX_RAYMARCHING; i++)
	{
		//calculate back step
		float step = dist * 2.0f;

		if(step < distThresh) step = distThresh;
		if(step > scan) 
		{
			step = scan;
			end = true;
		}
		//else if(step > 100.0f * distThresh) step = 100.0f * distThresh;
		scan -= step;
		
		float3 point = input->startPoint + input->viewVector * scan;
		
		dist = CalculateDistance(consts, point, input->calcParam).distance;
		
		if (consts->fractal.constantDEThreshold)
		{
			distThresh = consts->params.quality;
		}
		else
		{
			distThresh = scan * input->resolution * consts->params.persp / consts->params.quality;
		}
		
		//------------------- glow
		float glowOpacity = glow * step / input->depth;
		output = glowOpacity * glowColour + (1.0f - glowOpacity) * output;
		
		//----------------------- basic fog
		if(consts->params.fogEnabled)
		{
			float fogDensity = step / consts->params.fogVisibility;
			if(fogDensity > 1.0f) fogDensity = 1.0f;
			output = fogDensity * consts->params.fogColour + (1.0f - fogDensity) * output;
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
	
	
	int seed = i;
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
		for(blurIndex = 0; blurIndex<256; blurIndex++)
		{
		
			float randR = 0.003f * consts->params.DOFRadius*focus * sqrt(rand(&seed) / 65536.0 / 2.0f + 0.5f);
			float randAngle = rand(&seed);
			float randX = randR * sin(randAngle);
			float randZ = randR * cos(randAngle);
			#else
			float randX = 0.0f;
			float randZ = 0.0f;
			float focus = 1.0f;
			#endif //end DOFEnabled
		
			float3 back = (float3) {0.0f + randX/consts->params.zoom, 1.0f, 0.0f + randZ/consts->params.zoom} / consts->params.persp * consts->params.zoom;
			float3 start = consts->params.vp - Matrix33MulFloat3(rot, back);
			//printf("x %f, y %f, start %f %f %f\n", screenPoint.x, screenPoint.y, start.x, start.y, start.z);
			
			float aspectRatio = width / height;
			float x2,z2;
			x2 = (screenPoint.x / width - 0.5f) * aspectRatio;
			z2 = (screenPoint.y / height - 0.5f);
		
			float x3 = x2 + randX / focus / consts->params.persp;
			float z3 = z2 + randZ / focus / consts->params.persp;

			float3 viewVector = (float3) {x3 * consts->params.persp, 1.0f, z3 * consts->params.persp}; 
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
				cos(consts->params.mainLightAlfa - 0.5f * M_PI) * cos(-consts->params.mainLightBeta),
				sin(consts->params.mainLightAlfa - 0.5f * M_PI) * cos(-consts->params.mainLightBeta),
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

				shaderInputData.calcParam = &calcParam;
				shaderInputData.dist_thresh = reflectBuff[ray + local_offset].distThresh;
				
				if(consts->fractal.constantDEThreshold) shaderInputData.delta = reflectBuff[ray + local_offset].depth * resolution * consts->params.persp;
				else shaderInputData.delta = reflectBuff[ray + local_offset].distThresh * consts->params.quality;
				shaderInputData.lightVect = lightVector;
				shaderInputData.point = reflectBuff[ray + local_offset].point;
				shaderInputData.startPoint = reflectBuff[ray + local_offset].start;
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
					reflectBuff[ray + local_offset].depth = 1e20;
				}
				
				if (maxRay > 0 && rayEnd > 0 && ray != rayEnd)
				{
					colour = resultShader * consts->params.reflect; 
					colour += (1.0 - consts->params.reflect) * (objectShader + backgroudShader) + specular;
				}
				else
				{
					colour = objectShader + backgroudShader + specular;
				}
				resultShader = VolumetricShader(consts, &shaderInputData, colour);
				//resultShader = colour;
				
			}
			zBuff = reflectBuff[0 + local_offset].depth;
			#if _DOFEnabled
			totalColour += resultShader / 256.0f;
		}
		
		ushort R = convert_ushort_sat(totalColour.x * 65536.0f);
		ushort G = convert_ushort_sat(totalColour.y * 65536.0f);
		ushort B = convert_ushort_sat(totalColour.z * 65536.0f);
		#else
		ushort R = convert_ushort_sat(resultShader.x * 65536.0f);
		ushort G = convert_ushort_sat(resultShader.y * 65536.0f);
		ushort B = convert_ushort_sat(resultShader.z * 65536.0f);
		#endif //end DOFEnabled
		
		out[buffIndex].R = R;
		out[buffIndex].G = G;
		out[buffIndex].B = B;
		out[buffIndex].zBuffer = zBuff;
		
	}
}

