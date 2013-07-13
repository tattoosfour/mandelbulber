//#pragma OPENCL EXTENSION cl_amd_fp64 : enable

typedef float3 cl_float3;
typedef float cl_float;
typedef int cl_int;
typedef unsigned int cl_uint;
typedef unsigned short cl_ushort;

#include "cl_data.h"

#define MAX_RAYMARCHING 5000

typedef struct
{
	float3 z;
	float iters;
	float distance;
	float colourIndex;
} formulaOut;

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
		float delta = (index2 % 256)/256.0f;
		colOut = col1 + colDiff * delta;
	}
	return colOut;
}

float3 CalculateNormals(sClShaderInputData *input)
{
	float3 normal = 0.0f;
	//calculating normal vector based on distance estimation (gradient of distance function)
	if (!input->param->slowShading)
	{
		float delta = input->delta;
		//if(input.calcParam->interiorMode) delta = input.dist_thresh * input.param->doubles.quality * 0.2;

		float s1, s2, s3, s4;
		input->calcParam->N *= 5;
		//input.calcParam->minN = 0;

		bool maxIter;

		s1 = CalculateDistance(input->point, input->calcParam).distance;

		float3 deltax = (float3) {delta, 0.0f, 0.0f};
		s2 = CalculateDistance(input->point + deltax, input->calcParam).distance;

		float3 deltay = (float3) {0.0f, delta, 0.0f};
		s3 = CalculateDistance(input->point + deltay, input->calcParam).distance;

		float3 deltaz = (float3) {0.0f, 0.0f, delta};
		s4 = CalculateDistance(input->point + deltaz, input->calcParam).distance;

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

					float dist = CalculateDistance(point3, input->calcParam).distance;

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



float3 RayMarching(sClParams *param, sClFractal *calcParam, float3 start, float3 direction, float maxScan, bool binaryEnable, float *distThreshOut, float *lastDistOut, bool *foundOut, float *depthOut)
{
	float3 point;
	bool found = false;
	float scan = 1e-10f;
	float distThresh = *distThreshOut;
	float dist = 0.0f;
	float search_accuracy = 0.01f * param->quality;
	//printf("param->quality %f, start %f %f %f\n", param->quality, start.x, start.y, start.z);
	float search_limit = 1.0f - search_accuracy;
	float step = 1e-10f;
	float resolution = 1.0f/param->width;
	//*buffCount = 0;
	float distThreshInit = *distThreshOut;
	//printf("DE_factor %f\n", param->DEfactor);
	
	//printf("Start, start.x = %g, start.y = %g, start.z = %g\n", start.x, start.y, start.z);
	for (int i = 0; i < MAX_RAYMARCHING; i++)
	{
		point = start + direction * scan;
		//printf("viewVector %f %f %f\n", direction.x, direction.y, direction.z);
		//printf("scan %f\n", scan);
		//printf("DE_factor %f\n", param->DEfactor);
		
		bool max_iter = false;

		if (calcParam->constantDEThreshold)
		{
			distThresh = param->quality;
			//printf("DistThresh = %f\n", distThresh);
		}
		else
		{
			distThresh = scan * resolution * param->persp / param->quality + distThreshInit;
		}
		//calcParam->doubles.detailSize = distThresh;
		formulaOut outF;
		outF = CalculateDistance(point, calcParam);
		dist = outF.distance;
		
		//printf("Distance = %f\n", dist/distThresh);

		if (dist > 3.0f) dist = 3.0f;
		if (dist < distThresh)
		{
			found = true;
			break;
		}
		//printf("DE_factor %f\n", param->DEfactor);
		step = (dist - 0.5f * distThresh) * param->DEfactor;
		scan += step;
		if (scan > maxScan)
		{
			break;
		}

	}
	
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
			outF = CalculateDistance(point, calcParam);
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

float3 MainShadow(sClShaderInputData *input)
{
	float3 shadow = 1.0f;

	//starting point
	float3 point2;

	bool max_iter;
	float factor = input->delta / input->resolution;
	if (!input->param->penetratingLights) factor = input->param->viewDistanceMax;
	float dist = input->dist_thresh;

	float DE_factor = input->param->DEfactor;
	//if(input->param->imageSwitches.iterFogEnabled || input.param->imageSwitches.volumetricLightEnabled) DE_factor = 1.0;

	float start = input->delta;
	//if (input->calcParam->interiorMode) start = input.dist_thresh * DE_factor * 0.5;

	float opacity = 0.0f;
	float shadowTemp = 1.0f;

	float softRange = tan(input->param->shadowConeAngle / 180.0f * 3.14f);
	float maxSoft = 0.0f;

	//bool bSoft = (!input->param->imageSwitches.iterFogEnabled && !input.calcParam->limits_enabled && !input.calcParam->iterThresh) && softRange > 0.0;
	bool bSoft = softRange > 0.0f;
	
	for (float i = start; i < factor; i += dist * DE_factor)
	{
		point2 = input->point + input->lightVect * i;

		dist = CalculateDistance(point2, input->calcParam).distance;

		if (bSoft)
		{
			float angle = (dist - input->dist_thresh) / i;
			if (angle < 0.0f) angle = 0.0f;
			if (dist < input->dist_thresh) angle = 0.0f;
			float softShadow = (1.0f - angle / softRange);
			if (input->param->penetratingLights) softShadow *= (factor - i) / factor;
			if (softShadow < 0.0f) softShadow = 0.0f;
			if (softShadow > maxSoft) maxSoft = softShadow;
		}

		//if (input->param->imageSwitches.iterFogEnabled)
		//{
		//	opacity = IterOpacity(dist * DE_factor, input->calcParam->itersOut, input->calcParam->doubles.N, input->param->doubles.iterFogOpacityTrim,
		//			input.param->doubles.iterFogOpacity);
		//}
		//else
		//{
			opacity = 0.0f;
		//}
		shadowTemp -= opacity * (factor - i) / factor;

		if (dist < input->dist_thresh || shadowTemp < 0.0f)
		{
			shadowTemp -= (factor - i) / factor;
			if (!input->param->penetratingLights) shadowTemp = 0.0f;
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

float3 SurfaceColour(sClShaderInputData *input, global cl_float3 *palette)
{
	formulaOut outF = Fractal(input->point, input->calcParam);
	int colourNumber = outF.colourIndex * input->param->colouringSpeed + 256.0 * input->param->colouringOffset;
	float3 surfaceColour = 1.0;
	if (input->param->colouringEnabled) surfaceColour = IndexToColour(colourNumber, palette);
	return surfaceColour;
}

float3 ObjectShader(sClShaderInputData *input, float3 *specularOut, global cl_float3 *palette)
{
	float3 output;

	//normal vector
	float3 vn = CalculateNormals(input);
	input->normal = vn;
	
	float3 mainLight = input->param->mainLightIntensity * input->param->mainLightColour;
	
	//calculate shading based on angle of incidence
	float shadeTemp = dot(input->normal, input->lightVect);
	if (shadeTemp < 0.0f) shadeTemp = 0.0f;
	shadeTemp = input->param->mainLightIntensity * ((1.0f - input->param->shading) + input->param->shading * shadeTemp);
	float3 shade = shadeTemp;
	
	//calculate shadow
	float3 shadow = 1.0f;
	if(input->param->shadow) shadow = MainShadow(input);
	
	//calculate specular highlight
	float3 specular = MainSpecular(input);
	
	//calculate surface colour
	float3 colour = SurfaceColour(input, palette);
	
	
	output = mainLight * shadow * (shade * colour + specular);
	
	return output;
}

float3 BackgroundShader(sClShaderInputData *input)
{
	float3 vector = { 1.0, 1.0, -1.0 };
	vector = fast_normalize(vector);
	float grad = dot(input->viewVector, vector) + 1.0;
	float3 colour;
	if (grad < 1.0)
	{
		float ngrad = 1.0 - grad;
		colour = input->param->backgroundColour3 * ngrad + input->param->backgroundColour2 * grad;
	}
	else
	{
		grad = grad - 1.0;
		float ngrad = 1.0 - grad;
		colour = input->param->backgroundColour2 * ngrad + input->param->backgroundColour1 * grad;
	}
	return colour;
}

//------------------ MAIN RENDER FUNCTION --------------------
kernel void fractal3D(__global sClPixel *out, __global sClInBuff *inBuff, int Gcl_offset, __global sClReflect *reflectBuff)
{
	
	sClParams params = inBuff->params;
	sClFractal fractal = inBuff->fractal;
	int cl_offset = Gcl_offset;
	const unsigned int i = get_global_id(0) + cl_offset;
	const unsigned int imageX = i % params.width;
	const unsigned int imageY = i / params.width;
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
		//local sClStep stepBuff[MAX_RAYMARCHING];
		int buffCount;
		
		int maxRay = params.reflectionsMax;		


		float3 colour = 0.0f;
		
		
		
		//DOF effect
		#if _DOFEnabled
		int blurIndex = 0;
		float3 totalColour = (float3) {0.0f, 0.0f, 0.0f};
		float focus = params.DOFFocus;
		for(blurIndex = 0; blurIndex<256; blurIndex++)
		{
		
			float randR = 0.003f * params.DOFRadius*focus * sqrt(rand(&seed) / 65536.0 / 2.0f + 0.5f);
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
			x2 = (screenPoint.x / width - 0.5f) * aspectRatio;
			z2 = (screenPoint.y / height - 0.5f);
		
			float x3 = x2 + randX / focus / params.persp;
			float z3 = z2 + randZ / focus / params.persp;

			float3 viewVector = (float3) {x3 * params.persp, 1.0f, z3 * params.persp}; 
			viewVector = Matrix33MulFloat3(rot, viewVector);
			
			//ray-marching			
			float distThresh = 0.0f;
			float3 point = start;
			float3 startRay = start;
			float lastDist = 0.0f;
			bool found = false;
			float depth = 0.0f;
			
			float3 resultShader = 0.0f;
			float3 objectColour = 0.0f;
			
			float3 lightVector = (float3)
			{
				cos(params.mainLightAlfa - 0.5f * M_PI) * cos(-params.mainLightBeta),
				sin(params.mainLightAlfa - 0.5f * M_PI) * cos(-params.mainLightBeta),
				sin(-params.mainLightBeta)};
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

				//printf("Ray %d, DE_factor %f\n", ray, params.DEfactor);

				point = RayMarching(&params, &fractal, startRay, viewVector, params.viewDistanceMax, true, &distThresh, &lastDist, &found, &depth);
				
				//printf("point %f %f %f\n", point.x, point.y, point.z);
				
				reflectBuff[ray + local_offset].depth = depth;
				reflectBuff[ray + local_offset].found = found;
				reflectBuff[ray + local_offset].lastDist = lastDist;
				reflectBuff[ray + local_offset].point = point;
				reflectBuff[ray + local_offset].distThresh = distThresh;
				
				//printf("reflectBuff[ray].distThresh %f\n", reflectBuff[ray + local_offset].distThresh);
				
				//reflectBuff[ray].objectType = calcParam.objectOut;

				rayEnd = ray;
				if(!found) break;
				//if(reflectBuff[ray].reflect == 0) break;

				//calculate new ray direction and start point
				startRay = point;

				shaderInputData.calcParam = &fractal;
				shaderInputData.param = &params;
				shaderInputData.dist_thresh = distThresh;
				shaderInputData.lightVect = lightVector;
				shaderInputData.point = point;
				shaderInputData.viewVector = viewVector;
				//if(param.fractal.constantDEThreshold) shaderInputData.delta = depth * param.doubles.resolution * param.doubles.persp;
				//else 
				shaderInputData.delta = distThresh * params.quality;
				float3 vn = CalculateNormals(&shaderInputData);
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

				shaderInputData.calcParam = &fractal;
				shaderInputData.param = &params;
				shaderInputData.dist_thresh = reflectBuff[ray + local_offset].distThresh;
				
				//if(param.fractal.constantDEThreshold) delta = depth * resolution * params.persp;
				//else 
				shaderInputData.delta = reflectBuff[ray + local_offset].distThresh * params.quality;
				shaderInputData.lightVect = lightVector;
				shaderInputData.point = reflectBuff[ray + local_offset].point;
				shaderInputData.viewVector = reflectBuff[ray + local_offset].viewVector;
				shaderInputData.vectorsCount = params.AmbientOcclusionNoOfVectors;
				shaderInputData.lastDist = reflectBuff[ray + local_offset].lastDist;

				shaderInputData.depth = reflectBuff[ray + local_offset].depth;
				//shaderInputData.depth = depth;
				
				shaderInputData.resolution = resolution;
				//shaderInputData.vectorsAround = inBuff->vectorsAround;
				//shaderInputData.vectorsAroundColours = inBuff->vectorsAroundColours;
				//shaderInputData.envMappingTexture = param.envmapTexture;
				//shaderInputData.objectType = reflectBuff[ray].objectType;
				//shaderInputData.calcParam->doubles.detailSize = reflectBuff[ray].distThresh;

				if(reflectBuff[ray + local_offset].found)
				{
					//printf("Last dist %f = \n", lastDist / distThresh);	
					objectShader = ObjectShader(&shaderInputData, &specular, inBuff->palette);
				}
				else
				{
					backgroudShader = BackgroundShader(&shaderInputData);
					reflectBuff[ray + local_offset].depth = 1e20;
				}
				
				if (maxRay > 0 && rayEnd > 0 && ray != rayEnd)
				{
					colour = resultShader * params.reflect; 
					colour += (1.0 - params.reflect) * (objectShader + backgroudShader) + specular;
				}
				else
				{
					colour = objectShader + backgroudShader + specular;
				}
				resultShader = colour;

			}
			zBuff = reflectBuff[0 + local_offset].depth;
			#if _DOFEnabled
			totalColour += colour / 256.0f;
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

