/*
 * shaders.cpp
 *
 *  Created on: 2010-04-24
 *      Author: krzysztof marczak
 */

#include <cstdlib>

#include "shaders.h"
#include "interface.h"

sLight *Lights;
int lightsPlaced = 0;

sShaderOutput MainShadow(sParamRender *param, sFractal *calcParam, CVector3 point, CVector3 lightVect, double wsp_persp, double dist_thresh)
{
	sShaderOutput shadow = { 1.0, 1.0, 1.0 };

	//starting point
	CVector3 point2;

	bool max_iter;
	double factor = 1.0 * param->doubles.zoom * wsp_persp;
	if(!param->penetratingLights) factor = param->doubles.viewDistanceMax;
	double dist = dist_thresh;

	double start = dist_thresh;
	if(calcParam->interiorMode) 
		start = dist_thresh*param->doubles.DE_factor*0.5;

	double opacity = 0.0;
	double shadowTemp = 1.0;

	for (double i = start; i < factor; i += dist * param->doubles.DE_factor)
	{
		point2 = point + lightVect * i;

		dist = CalculateDistance(point2, *calcParam, &max_iter);

		if (param->fractal.iterThresh)
		{
			if (dist < dist_thresh && !max_iter)
			{
				dist = dist_thresh * 1.01;
			}
		}

		if(param->imageSwitches.iterFogEnabled)
		{
			opacity = IterOpacity(dist * param->doubles.DE_factor,calcParam->itersOut, calcParam->doubles.N, param->doubles.iterFogOpacityTrim, param->doubles.iterFogOpacity);
		}
		else
		{
			opacity = 0.0;
		}
		shadowTemp -= opacity * (factor - i) / factor;

		if (dist < dist_thresh || max_iter || shadowTemp < 0.0)
		{
			shadowTemp -= (factor - i) / factor;
			if(!param->penetratingLights) shadowTemp = 0.0;
			if(shadowTemp < 0.0) shadowTemp = 0.0;
			break;
		}
	}
	shadow.R = shadowTemp;
	shadow.G = shadowTemp;
	shadow.B = shadowTemp;
	return shadow;
}

sShaderOutput FastAmbientOcclusion(sFractal *calcParam, CVector3 point)
{
	sShaderOutput output;
	double min_radius = Compute<fake_AO>(point, *calcParam);
	double j = (min_radius - 0.65) * 1.0; //0.65
	if (j > 1.0) j = 1.0;
	if (j < 0) j = 0;
	output.R = j;
	output.G = j;
	output.B = j;
	return output;
}

sShaderOutput FastAmbientOcclusion2(sFractal *calcParam, CVector3 point, CVector3 normal, double dist_thresh, double tune, int quality)
{
	//reference: http://www.iquilezles.org/www/material/nvscene2008/rwwtt.pdf (Iñigo Quilez – iq/rgba)

	double delta = dist_thresh;
	double aoTemp = 0;
	for(int i=1; i<quality*quality; i++)
	{
		double scan = i*i*delta;
		CVector3 pointTemp = point + normal * scan;
		bool max_iter;
		double dist = CalculateDistance(pointTemp, *calcParam, &max_iter);
		aoTemp += 1.0/(pow(2.0,i)) * (scan - tune*dist)/dist_thresh;
		//if(dist < 0.5*dist_thresh) break;
	}
	double ao = 1.0 - 0.2*aoTemp;
	if(ao < 0) ao = 0;
	sShaderOutput output = {ao,ao,ao};
	return output;
}

sShaderOutput AmbientOcclusion(sParamRender *param, sFractal *calcParam, CVector3 point, double wsp_persp, double dist_thresh, double last_distance, sVectorsAround *vectorsAround,
		int vectorsCount)
{
	sShaderOutput AO = { 0, 0, 0 };

	double delta = param->doubles.resolution * param->doubles.zoom * wsp_persp;

	bool max_iter;
	double start_dist = dist_thresh;
	double end_dist = param->doubles.zoom * wsp_persp;
	double intense = 0;
	if (start_dist < delta) start_dist = delta;
	for (int i = 0; i < vectorsCount; i++)
	{
		sVectorsAround v = vectorsAround[i];

		double dist = last_distance;

		double opacity = 0.0;
		double shadowTemp = 1.0;

		for (double r = start_dist; r < end_dist; r += dist * 2.0)
		{
			CVector3 point2 = point + v.v * r;

			dist = CalculateDistance(point2, *calcParam, &max_iter);
			if (param->fractal.iterThresh)
			{
				if (dist < dist_thresh && !max_iter)
				{
					dist = dist_thresh * 1.01;
				}
			}

			if (param->imageSwitches.iterFogEnabled)
			{
				opacity = IterOpacity(dist * param->doubles.DE_factor, calcParam->itersOut, calcParam->doubles.N, param->doubles.iterFogOpacityTrim, param->doubles.iterFogOpacity);
			}
			else
			{
				opacity = 0.0;
			}
			shadowTemp -= opacity * (end_dist - r) / end_dist;

			if (dist < dist_thresh || max_iter || shadowTemp < 0.0)
			{
				shadowTemp -= (end_dist - r) / end_dist;
				if(shadowTemp < 0.0) shadowTemp = 0.0;
				break;
			}
		}

		intense = shadowTemp;

		AO.R += intense * v.R;
		AO.G += intense * v.G;
		AO.B += intense * v.B;

	}
	AO.R /= (vectorsCount * 256.0);
	AO.G /= (vectorsCount * 256.0);
	AO.B /= (vectorsCount * 256.0);

	return AO;
}

CVector3 CalculateNormals(sParamRender *param, sFractal *calcParam, CVector3 point, double dist_thresh)
{
	CVector3 normal(0, 0, 0);
	//calculating normal vector based on distance estimation (gradient of distance function)
	if (!param->slowShading)
	{
		//calcParam->DE_threshold = 0;
		//double delta = param->resolution * param->zoom * wsp_persp;
		double delta = dist_thresh * param->doubles.smoothness;
		if(calcParam->interiorMode) delta = dist_thresh * 0.2;

		double s1, s2, s3, s4;
		calcParam->doubles.N = param->fractal.doubles.N * 2;
		calcParam->minN = 0;

		s1 = CalculateDistance(point, *calcParam);

		CVector3 deltax(delta, 0.0, 0.0);
		s2 = CalculateDistance(point + deltax, *calcParam);

		CVector3 deltay(0.0, delta, 0.0);
		s3 = CalculateDistance(point + deltay, *calcParam);

		CVector3 deltaz(0.0, 0.0, delta);
		s4 = CalculateDistance(point + deltaz, *calcParam);

		normal.x = s2 - s1;
		normal.y = s3 - s1;
		normal.z = s4 - s1;
		calcParam->doubles.N = param->fractal.doubles.N;
		calcParam->minN = param->fractal.minN;
		//calcParam->DE_threshold = DEthreshold_temp;
	}

	//calculating normal vector based on average value of binary central difference
	else
	{
		double result2;
		bool max_iter;

		CVector3 point2;
		CVector3 point3;
		for (point2.x = -1.0; point2.x <= 1.0; point2.x += 0.2) //+0.2
		{
			for (point2.y = -1.0; point2.y <= 1.0; point2.y += 0.2)
			{
				for (point2.z = -1.0; point2.z <= 1.0; point2.z += 0.2)
				{
					point3 = point + point2 * dist_thresh * param->doubles.smoothness;

					double dist = CalculateDistance(point3, *calcParam, &max_iter);

					if (param->fractal.iterThresh)
					{
						if (dist < dist_thresh && !max_iter)
						{
							dist = dist_thresh * 1.01;
						}
					}
					if (dist < dist_thresh || max_iter) result2 = 0.0;
					else result2 = 1.0;

					normal += (point2 * result2);
				}
			}
		}
	}

	if(normal.x == 0 && normal.y == 0 && normal.z == 0)
	{
		normal.x = 1;
	}
	else
	{
		normal.Normalize();
	}

	return normal;
}

sShaderOutput MainShading(CVector3 normal, CVector3 lightVector)
{
	sShaderOutput shading;
	double shade = normal.Dot(lightVector);
	if (shade < 0) shade = 0;
	shading.R = shade;
	shading.G = shade;
	shading.B = shade;
	return shading;
}

sShaderOutput MainSpecular(CVector3 normal, CVector3 lightVector, CVector3 viewVector)
{
	sShaderOutput specular;
	CVector3 half = lightVector - viewVector;
	half.Normalize();
	double shade2 = normal.Dot(half);
	if (shade2 < 0.0) shade2 = 0.0;
	shade2 = pow(shade2, 30.0) * 1.0;
	if (shade2 > 15.0) shade2 = 15.0;
	specular.R = shade2;
	specular.G = shade2;
	specular.B = shade2;
	return specular;
}

sShaderOutput EnvMapping(CVector3 normal, CVector3 viewVector, cTexture *texture)
{
	sShaderOutput envReflect;
	CVector3 reflect;
	double dot = -viewVector.Dot(normal);
	reflect = normal * 2.0 * dot + viewVector;

	double alfaTexture = reflect.GetAlfa() + M_PI;
	double betaTexture = reflect.GetBeta();
	double texWidth = texture->Width();
	double texHeight = texture->Height();

	if (betaTexture > 0.5 * M_PI) betaTexture = 0.5 * M_PI - betaTexture;

	if (betaTexture < -0.5 * M_PI) betaTexture = -0.5 * M_PI + betaTexture;

	double dtx = (alfaTexture / (2.0 * M_PI)) * texWidth + texWidth * 8.25;
	double dty = (betaTexture / (M_PI) + 0.5) * texHeight + texHeight * 8.0;
	dtx = fmod(dtx, texWidth);
	dty = fmod(dty, texHeight);
	if (dtx < 0) dtx = 0;
	if (dty < 0) dty = 0;
	envReflect.R = texture->Pixel(dtx, dty).R / 256.0;
	envReflect.G = texture->Pixel(dtx, dty).G / 256.0;
	envReflect.B = texture->Pixel(dtx, dty).B / 256.0;
	return envReflect;
}

int SurfaceColour(sFractal *calcParam, CVector3 point)
{
	calcParam->doubles.N *= 10;
	int nrKol = floor(Compute<colouring> (point, *calcParam));
	calcParam->doubles.N /= 10;
	nrKol = abs(nrKol) % (248*256);
	return nrKol;
}

sShaderOutput TexturedBackground(sParamRender *param, CVector3 viewVector)
{
	sShaderOutput pixel2;
	if (param->textured_background)
	{
		if(param->background_as_fulldome)
		{
			double alfaTexture = viewVector.GetAlfa();
			double betaTexture = viewVector.GetBeta();
			int texWidth = param->backgroundTexture->Width()*0.5;
			int texHeight = param->backgroundTexture->Height();
			int offset = 0;

			if(betaTexture < 0)
			{
				betaTexture = -betaTexture;
				alfaTexture += M_PI;
				offset = texWidth;
			}
			double texX = 0.5 * texWidth + cos(alfaTexture) * (1.0 - betaTexture / (0.5 * M_PI)) * texWidth * 0.5 + offset;
			double texY = 0.5 * texHeight + sin(alfaTexture) * (1.0 - betaTexture / (0.5 * M_PI)) * texHeight * 0.5;
			sRGB8 pixel = param->backgroundTexture->Pixel(texX, texY);
			pixel2.R = pixel.R / 256.0;
			pixel2.G = pixel.G / 256.0;
			pixel2.B = pixel.B / 256.0;
		}
		else
		{
			double alfaTexture = fmod(viewVector.GetAlfa() + 2.5 * M_PI, 2 * M_PI);
			double betaTexture = viewVector.GetBeta();
			if (betaTexture > 0.5 * M_PI) betaTexture = 0.5 * M_PI - betaTexture;
			if (betaTexture < -0.5 * M_PI) betaTexture = -0.5 * M_PI + betaTexture;
			double texX = alfaTexture / (2.0 * M_PI) * param->backgroundTexture->Width();
			double texY = (betaTexture / (M_PI) + 0.5) * param->backgroundTexture->Height();
			sRGB8 pixel = param->backgroundTexture->Pixel(texX, texY);
			pixel2.R = pixel.R/256.0;
			pixel2.G = pixel.G/256.0;
			pixel2.B = pixel.B/256.0;
		}
	}
	else
	{
		CVector3 vector(0.5, 0.5, -0.5);
		vector.Normalize();
		double grad = (viewVector.Dot(vector)+1.0);
		sRGB16 pixel;
		if(grad < 1)
		{
			double Ngrad = 1.0 - grad;
			pixel.R = (param->background_color3.R * Ngrad + param->background_color2.R * grad);
			pixel.G = (param->background_color3.G * Ngrad + param->background_color2.G * grad);
			pixel.B = (param->background_color3.B * Ngrad + param->background_color2.B * grad);
		}
		else
		{
			grad = grad - 1;
			double Ngrad = 1.0 - grad;
			pixel.R = (param->background_color2.R * Ngrad + param->background_color1.R * grad);
			pixel.G = (param->background_color2.G * Ngrad + param->background_color1.G * grad);
			pixel.B = (param->background_color2.B * Ngrad + param->background_color1.B * grad);
		}

		pixel2.R = pixel.R/65536.0;
		pixel2.G = pixel.G/65536.0;
		pixel2.B = pixel.B/65536.0;
	}
	return pixel2;
}

sShaderOutput LightShading(sParamRender *fractParams, sFractal *calcParam, CVector3 point, CVector3 normal, CVector3 viewVector, sLight light, double wsp_persp,
		double dist_thresh, int number, sShaderOutput *outSpecular)
{
	sShaderOutput shading;

	CVector3 d = light.position - point;

	double distance = d.Length();

	//angle of incidence
	CVector3 lightVector = d;
	lightVector.Normalize();

	double intensity = fractParams->doubles.auxLightIntensity * 100.0 * light.intensity / (distance * distance) / number;
	double shade = normal.Dot(lightVector);
	if (shade < 0) shade = 0;
	shade = shade * intensity;
	if (shade > 5.0) shade = 5.0;

	//specular
	CVector3 half = lightVector - viewVector;
	half.Normalize();
	double shade2 = normal.Dot(half);
	if (shade2 < 0.0) shade2 = 0.0;
	shade2 = pow(shade2, 30.0) * 1.0;
	shade2 *= intensity * fractParams->doubles.imageAdjustments.specular;
	if (shade2 > 15.0) shade2 = 15.0;

	//calculate shadow
	if ((shade > 0.01 || shade2 > 0.01) && fractParams->shadow)
	{
		double light = AuxShadow(fractParams, calcParam, wsp_persp, dist_thresh, distance, point, lightVector, fractParams->penetratingLights);
		shade *= light;
		shade2 *= light;
	}
	else
	{
		if(fractParams->shadow)
		{
			shade = 0;
			shade2 = 0;
		}
	}

	shading.R = shade * light.colour.R / 65536.0;
	shading.G = shade * light.colour.G / 65536.0;
	shading.B = shade * light.colour.B / 65536.0;

	outSpecular->R = shade2 * light.colour.R / 65536.0;
	outSpecular->G = shade2 * light.colour.G / 65536.0;
	outSpecular->B = shade2 * light.colour.B / 65536.0;

	return shading;
}

sShaderOutput AuxLightsShader(sParamRender *param, sFractal *calcParam, CVector3 point, CVector3 normal, CVector3 viewVector, double wsp_persp, double dist_thresh,
		sShaderOutput *specularAuxOut)
{
	int numberOfLights = lightsPlaced;
	if (numberOfLights < 4) numberOfLights = 4;
	sShaderOutput shadeAuxSum = { 0, 0, 0 };
	sShaderOutput specularAuxSum = { 0, 0, 0 };
	for (int i = 0; i < numberOfLights; i++)
	{
		if (i < param->auxLightNumber || Lights[i].enabled)
		{
			sShaderOutput specularAuxOutTemp;
			sShaderOutput shadeAux = LightShading(param, calcParam, point, normal, viewVector, Lights[i], wsp_persp, dist_thresh, numberOfLights, &specularAuxOutTemp);
			shadeAuxSum.R += shadeAux.R;
			shadeAuxSum.G += shadeAux.G;
			shadeAuxSum.B += shadeAux.B;
			specularAuxSum.R += specularAuxOutTemp.R;
			specularAuxSum.G += specularAuxOutTemp.G;
			specularAuxSum.B += specularAuxOutTemp.B;
		}
	}
	*specularAuxOut = specularAuxSum;
	return shadeAuxSum;
}

double AuxShadow(sParamRender *fractParams, sFractal *calcParam, double wsp_persp, double dist_thresh, double distance, CVector3 point, CVector3 lightVector, bool linear)
{
	double delta = fractParams->doubles.resolution * fractParams->doubles.zoom * wsp_persp;
	double stepFactor = 1.0;
	double step = delta * fractParams->doubles.DE_factor * stepFactor;
	double dist = step;
	double light = 1.0;

	double opacity = 0.0;
	double shadowTemp = 1.0;

	for (double i = dist_thresh; i < distance; i += dist * stepFactor * fractParams->doubles.DE_factor)
	{
		CVector3 point2 = point + lightVector * i;

		bool max_iter;
		dist = CalculateDistance(point2, *calcParam, &max_iter);
		if (fractParams->fractal.iterThresh)
		{
			if (dist < dist_thresh && !max_iter)
			{
				dist = dist_thresh * 1.01;
			}
		}

		if(fractParams->imageSwitches.iterFogEnabled)
		{
			opacity = IterOpacity(dist * stepFactor * fractParams->doubles.DE_factor, calcParam->itersOut, calcParam->doubles.N, fractParams->doubles.iterFogOpacityTrim, fractParams->doubles.iterFogOpacity);
		}
		else
		{
			opacity = 0.0;
		}
		shadowTemp -= opacity * (distance - i) / distance;

		if (dist < 0.5 * dist_thresh || max_iter || shadowTemp < 0.0)
		{
			if(linear)
			{
				shadowTemp -= (distance - i) / distance;
				if(shadowTemp < 0.0) shadowTemp = 0.0;
			}
			else
			{
				shadowTemp = 0.0;
			}
			break;
		}
	}
	light = shadowTemp;
	return light;
}

sShaderOutput VolumetricLight(sParamRender *param, sFractal *calcParam, CVector3 point, double yStart, double min_y, double last_distance, double zoom, CVector3 lightVector)
{
	//volumetric light
	double tempDist = last_distance;
	sShaderOutput volFog;
	volFog.R = volFog.G = volFog.B = 0.0;
	enumPerspectiveType perspectiveType = param->perspectiveType;
	double fov = param->doubles.persp;
	double quality = param->doubles.quality;
	CVector3 vp = param->doubles.vp;
	CRotationMatrix mRot;
	mRot.RotateZ(param->doubles.alfa);
	mRot.RotateX(param->doubles.beta);
	mRot.RotateY(param->doubles.gamma);
	bool max_iter;
	double DE_factor = param->doubles.DE_factor;
	double resolution = param->doubles.resolution;

	for (double scan = yStart; scan > min_y; scan -= tempDist / zoom)
	{
		CVector3 pointTemp = Projection3D(CVector3(point.x, scan, point.z), vp, mRot, perspectiveType, fov, zoom);
		double wsp_perspTemp = 1.0 + scan * fov;
		double dist_threshTemp = zoom * resolution * wsp_perspTemp / quality;
		tempDist = CalculateDistance(pointTemp, *calcParam, &max_iter) * DE_factor / param->doubles.volumetricLightQuality + dist_threshTemp;
		tempDist = tempDist * (1.0 - Random(1000) / 2000.0);

		for (int i = 0; i < 5; i++)
		{
			if (i == 0 && param->volumetricLightEnabled[0])
			{
				sShaderOutput shadowOutputTemp = MainShadow(param, calcParam, pointTemp, lightVector, wsp_perspTemp, dist_threshTemp);
				volFog.R += 100.0 * shadowOutputTemp.R * tempDist * param->doubles.volumetricLightIntensity[0] * param->effectColours.mainLightColour.R / 65536.0;
				volFog.G += 100.0 * shadowOutputTemp.G * tempDist * param->doubles.volumetricLightIntensity[0] * param->effectColours.mainLightColour.G / 65536.0;
				volFog.B += 100.0 * shadowOutputTemp.B * tempDist * param->doubles.volumetricLightIntensity[0] * param->effectColours.mainLightColour.B / 65536.0;
			}
			if (i > 0)
			{
				if (Lights[i - 1].enabled && param->volumetricLightEnabled[i])
				{
					CVector3 d = Lights[i - 1].position - pointTemp;
					double distance = d.Length();
					double distance2 = distance * distance;
					CVector3 lightVectorTemp = d;
					lightVectorTemp.Normalize();
					double light = AuxShadow(param, calcParam, wsp_perspTemp, dist_threshTemp, distance, pointTemp, lightVectorTemp, param->penetratingLights);
					volFog.R += 1000.0 * light * Lights[i - 1].colour.R / 65536.0 * param->doubles.volumetricLightIntensity[i] * tempDist / distance2;
					volFog.G += 1000.0 * light * Lights[i - 1].colour.G / 65536.0 * param->doubles.volumetricLightIntensity[i] * tempDist / distance2;
					volFog.B += 1000.0 * light * Lights[i - 1].colour.B / 65536.0 * param->doubles.volumetricLightIntensity[i] * tempDist / distance2;
				}
			}
		}
	}
	return volFog;
}

sShaderOutput VolumetricFog(sParamRender *param, int buffCount, double *distanceBuff, double *stepBuff, double *densityOut)
{
	sShaderOutput volFog;
	volFog.R = volFog.G = volFog.B = 0.0;
	double density = 0;
	double fogR = param->fogColour1.R;
	double fogG = param->fogColour1.G;
	double fogB = param->fogColour1.B;
	double colourThresh = param->doubles.fogColour1Distance;
	double colourThresh2 = param->doubles.fogColour2Distance;
	double fogReduce = param->doubles.fogDistanceFactor;
	double fogIntensity = param->doubles.fogDensity;
	for (int i = buffCount - 2; i >= 0; i--)
	{
		double densityTemp = (stepBuff[i] * fogReduce) / (distanceBuff[i] * distanceBuff[i] + fogReduce * fogReduce);

		double k = distanceBuff[i] / colourThresh;
		if (k > 1) k = 1.0;
		double kn = 1.0 - k;
		double fogRtemp = (param->fogColour1.R * kn + param->fogColour2.R * k);
		double fogGtemp = (param->fogColour1.G * kn + param->fogColour2.G * k);
		double fogBtemp = (param->fogColour1.B * kn + param->fogColour2.B * k);

		double k2 = distanceBuff[i] / colourThresh2 * k;
		if (k2 > 1) k2 = 1.0;
		kn = 1.0 - k2;
		fogRtemp = (fogRtemp * kn + param->fogColour3.R * k2);
		fogGtemp = (fogGtemp * kn + param->fogColour3.G * k2);
		fogBtemp = (fogBtemp * kn + param->fogColour3.B * k2);

		double d1 = fogIntensity * densityTemp / (1.0 + fogIntensity * densityTemp);
		double d2 = 1.0 - d1;
		fogR = fogR * d2 + fogRtemp * d1;
		fogG = fogG * d2 + fogGtemp * d1;
		fogB = fogB * d2 + fogBtemp * d1;

		density += densityTemp;
	}
	density *= fogIntensity;
	volFog.R = fogR;
	volFog.G = fogG;
	volFog.B = fogB;

	*densityOut = density;
	return volFog;
}

void PlaceRandomLights(sParamRender *fractParams, bool onlyPredefined)
{
	srand(fractParams->auxLightRandomSeed);

	if(!onlyPredefined)
	{
		delete[] Lights;
		Lights = new sLight[fractParams->auxLightNumber + 4];
	}
	sFractal calcParam = fractParams->fractal;
	bool max_iter;

	int trial_number = 0;
	double radius_multiplier = 1.0;


	if(!onlyPredefined)
	{
	for (int i = 0; i < fractParams->auxLightNumber; i++)
		{
			trial_number++;

			CVector3 random;
			random.x = (Random(2000000) / 1000000.0 - 1.0) + (Random(1000000) / 1.0e12);
			random.y = (Random(2000000) / 1000000.0 - 1.0) + (Random(1000000) / 1.0e12);
			random.z = (Random(2000000) / 1000000.0 - 1.0) + (Random(1000000) / 1.0e12);

			CVector3 position = fractParams->doubles.auxLightRandomCenter + random * fractParams->doubles.auxLightDistributionRadius * radius_multiplier;

			double distance = CalculateDistance(position, calcParam, &max_iter);

			if (trial_number > 1000)
			{
				radius_multiplier *= 1.1;
				trial_number = 0;
			}

			if (distance > 0 && !max_iter && distance < fractParams->doubles.auxLightMaxDist * radius_multiplier)
			{
				radius_multiplier = 1.0;

				Lights[i].position = position;

				sRGB colour = { 20000 + Random(80000), 20000 + Random(80000), 20000 + Random(80000) };
				double maxColour = dMax(colour.R, colour.G, colour.B);
				colour.R = colour.R * 65536.0 / maxColour;
				colour.G = colour.G * 65536.0 / maxColour;
				colour.B = colour.B * 65536.0 / maxColour;
				Lights[i].colour = colour;

				Lights[i].intensity = distance * distance / fractParams->doubles.auxLightMaxDist;
				Lights[i].enabled = true;
				printf("Light no. %d: x=%f, y=%f, z=%f, distance=%f\n", i, position.x, position.y, position.z, distance);
			}
			else
			{
				i--;
			}
		}
	}

	if (fractParams->auxLightPre1Enabled)
	{
		Lights[0].position = fractParams->doubles.auxLightPre1;
		Lights[0].intensity = fractParams->doubles.auxLightPre1intensity;
		Lights[0].colour = fractParams->auxLightPre1Colour;
		Lights[0].enabled = true;
	}
	else
	{
		Lights[0].enabled = false;
	}

	if (fractParams->auxLightPre2Enabled)
	{
		Lights[1].position = fractParams->doubles.auxLightPre2;
		Lights[1].intensity = fractParams->doubles.auxLightPre2intensity;
		Lights[1].colour = fractParams->auxLightPre2Colour;
		Lights[1].enabled = true;
	}
	else
	{
		Lights[1].enabled = false;
	}

	if (fractParams->auxLightPre3Enabled)
	{
		Lights[2].position = fractParams->doubles.auxLightPre3;
		Lights[2].intensity = fractParams->doubles.auxLightPre3intensity;
		Lights[2].colour = fractParams->auxLightPre3Colour;
		Lights[2].enabled = true;
	}
	else
	{
		Lights[2].enabled = false;
	}

	if (fractParams->auxLightPre4Enabled)
	{
		Lights[3].position = fractParams->doubles.auxLightPre4;
		Lights[3].intensity = fractParams->doubles.auxLightPre4intensity;
		Lights[3].colour = fractParams->auxLightPre4Colour;
		Lights[3].enabled = true;
	}
	else
	{
		Lights[3].enabled = false;
	}

	lightsPlaced = fractParams->auxLightNumber;
	if (lightsPlaced < 4) lightsPlaced = 4;
}

void PostRenderingLights(cImage *image, sParamRender *fractParam)
{
	int width = image->GetWidth();
	int height = image->GetHeight();
	double aspectRatio = (double) width / height;

	//preparing rotation matrix
	CRotationMatrix mRot;
	mRot.RotateY(-fractParam->doubles.gamma);
	mRot.RotateX(-fractParam->doubles.beta);
	mRot.RotateZ(-fractParam->doubles.alfa);

	CVector3 point3D1, point3D2;

	int numberOfLights = lightsPlaced;

	int tiles = fractParam->noOfTiles;
	int tile = fractParam->tileCount;

	double tileXOffset = (tile % tiles);
	double tileYOffset = (tile / tiles);

	if (numberOfLights < 4) numberOfLights = 4;

	for (int i = 0; i < numberOfLights; ++i)
	{
		if(programClosed) break;

		if (i < fractParam->auxLightNumber || Lights[i].enabled)
		{
			point3D1 = Lights[i].position - fractParam->doubles.vp;
			point3D2 = mRot.RotateVector(point3D1);
			double y2 = point3D2.y;
			double y = y2 / fractParam->doubles.zoom;
			double wsp_persp = 1.0 + y * fractParam->doubles.persp;
			double x2 = point3D2.x / wsp_persp;
			double z2 = point3D2.z / wsp_persp;
			double x = (x2 / (fractParam->doubles.zoom * aspectRatio) + 0.5 - tileXOffset/tiles) * width * tiles;
			double z = (z2 / fractParam->doubles.zoom + 0.5 - tileYOffset/tiles) * height * tiles;

			int xs = (int) x;
			int ys = (int) z;

			if (xs >= -width*0.3 && xs < width*1.3 && ys >= -width*0.3 && ys < height*1.3)
			{

				if (y > (-1.0 / fractParam->doubles.persp))
				{
					int R = Lights[i].colour.R;
					int G = Lights[i].colour.G;
					int B = Lights[i].colour.B;

					double size = 50.0 / wsp_persp * Lights[i].intensity / fractParam->doubles.zoom * width * tiles * fractParam->doubles.auxLightIntensity / fractParam->auxLightNumber
							* fractParam->doubles.auxLightVisibility;

					int x_start = xs - size * 5 - 1;
					int x_end = xs + size * 5 + 1;
					int y_start = ys - size * 5 - 1;
					int y_end = ys + size * 5 + 1;

					if (x_start < 0) x_start = 0;
					if (x_end >= width - 1) x_end = width - 1;
					if (y_start < 0) y_start = 0;
					if (y_end >= height - 1) y_end = height - 1;

					for (int yy = y_start; yy <= y_end; yy++)
					{
						for (int xx = x_start; xx <= x_end; xx++)
						{
							double dx = xx - x;
							double dy = yy - z;
							double r = sqrt(dx * dx + dy * dy) / size;

							double r2 = sqrt(dx * dx + dy * dy);
							if (r2 > size * 5) r2 = size * 5;
							double bellFunction = (cos(r2 * M_PI/(size*5.0)) + 1.0) / 2.0;

							double bright = bellFunction / (r * r);

							double coveringFactor1 = 1.0;

							for(double rr=0; rr<r2; rr+=1.0)
							{
								double xtemp = dx/r2 * rr + x;
								double ytemp = dy/r2 * rr + z;
								if (xtemp < 0) xtemp = 0;
								if (xtemp >= width - 1) xtemp = width - 1;
								if (ytemp < 0) ytemp = 0;
								if (ytemp >= height - 1) ytemp = height - 1;
								double tempz = image->GetPixelZBuffer((int)xtemp,(int)ytemp);
								if(tempz < y)
								{
									coveringFactor1 = rr/r2*0.9;
									break;
								}
							}

							double coveringFactor2 = 0.1;
							if(image->GetPixelZBuffer(xx,yy) < y)
							{
								coveringFactor2 = 0;
							}

							bright*=(coveringFactor1 + coveringFactor2);

							if (bright > 10.0) bright = 10.0;

							sRGB16 oldPixel = image->GetPixelImage(xx,yy);
							int oldAlpha = image->GetPixelAlpha(xx,yy);
							int newR = oldPixel.R + bright * R;
							int newG = oldPixel.G + bright * G;
							int newB = oldPixel.B + bright * B;
							if (newR > 65535) newR = 65535;
							if (newG > 65535) newG = 65535;
							if (newB > 65535) newB = 65535;

							sRGB16 pixel = { newR, newG, newB };
							int alpha = oldAlpha + bright * 65536;
							if (alpha > 65535) alpha = 65535;

							image->PutPixelImage(xx,yy,pixel);
							image->PutPixelAlpha(xx,yy,alpha);
						}
					}
				}
			}
			char progressText[1000];
			double percent_done = (double) (i+1) / numberOfLights * 100.0;
			sprintf(progressText, "Rendering visible lights. Done %.1f%%", percent_done);
			if(image->IsPreview())
			{
				gtk_progress_bar_set_text(GTK_PROGRESS_BAR(Interface.progressBar), progressText);
				while (gtk_events_pending())
					gtk_main_iteration();
			}
		}
	}
}

double IterOpacity(double step, double iters, double maxN, double trim, double opacitySp)
{
	double opacity = ((double)iters - trim) / maxN;
	if(opacity < 0.0) opacity = 0;
	opacity*=opacity;
	opacity*=step  * opacitySp;
	if(opacity > 1.0) opacity = 1.0;
	return opacity;
}

sShaderOutput IterationFog(sParamRender *param, sFractal *calcParam, CVector3 point, double yStart, double min_y, double last_distance, double zoom,
		CVector3 lightVector, bool found, double *densityOut, sVectorsAround *vectorsAround, int vectorsCount, sRGB16 oldPixel)
{
	double density = 0.0;
	double tempDist = last_distance;
	sShaderOutput iterFog = {oldPixel.R/65536.0, oldPixel.G/65536.0, oldPixel.B/65536.0};
	enumPerspectiveType perspectiveType = param->perspectiveType;
	double fov = param->doubles.persp;
	double quality = param->doubles.quality;
	CVector3 vp = param->doubles.vp;
	CRotationMatrix mRot;
	mRot.RotateZ(param->doubles.alfa);
	mRot.RotateX(param->doubles.beta);
	mRot.RotateY(param->doubles.gamma);
	bool max_iter;
	double DE_factor = param->doubles.DE_factor;
	double resolution = param->doubles.resolution;

	for (double scan = yStart; scan > min_y; scan -= tempDist / zoom)
	{
		CVector3 pointTemp = Projection3D(CVector3(point.x, scan, point.z), vp, mRot, perspectiveType, fov, zoom);
		double wsp_perspTemp = 1.0 + scan * fov;
		double dist_threshTemp = zoom * resolution * wsp_perspTemp / quality;
		tempDist = CalculateDistance(pointTemp, *calcParam, &max_iter) * DE_factor / param->doubles.volumetricLightQuality + dist_threshTemp;
		tempDist = tempDist * (1.0 - Random(1000) / 2000.0);

		int L = calcParam->itersOut;
		double opacity = IterOpacity(tempDist, L, param->fractal.doubles.N, param->doubles.iterFogOpacityTrim, param->doubles.iterFogOpacity);

		sShaderOutput newColour = { 0.0, 0.0, 0.0 };

		if (opacity > 0)
		{
			//fog colour
			double iterFactor = (double)2.0* (L - param->doubles.iterFogOpacityTrim)/(param->fractal.doubles.N - param->doubles.iterFogOpacityTrim);
			double k = iterFactor;
			if (k > 1) k = 1.0;
			double kn = 1.0 - k;
			double fogColR = (param->fogColour1.R * kn + param->fogColour2.R * k);
			double fogColG = (param->fogColour1.G * kn + param->fogColour2.G * k);
			double fogColB = (param->fogColour1.B * kn + param->fogColour2.B * k);

			double k2 = iterFactor - 1.0;
			if (k2 < 0.0) k2 = 0.0;
			if (k2 > 1.0) k2 = 1.0;
			kn = 1.0 - k2;
			fogColR = (fogColR * kn + param->fogColour3.R * k2);
			fogColG = (fogColG * kn + param->fogColour3.G * k2);
			fogColB = (fogColB * kn + param->fogColour3.B * k2);
			//----

			for (int i = 0; i < 5; i++)
			{
				if (i == 0)
				{
					if(param->doubles.imageAdjustments.mainLightIntensity * param->doubles.imageAdjustments.directLight > 0.0)
					{
						sShaderOutput shadowOutputTemp = MainShadow(param, calcParam, pointTemp, lightVector, wsp_perspTemp, dist_threshTemp);
						newColour.R += shadowOutputTemp.R * param->effectColours.mainLightColour.R / 65536.0 * param->doubles.imageAdjustments.mainLightIntensity * param->doubles.imageAdjustments.directLight;
						newColour.G += shadowOutputTemp.G * param->effectColours.mainLightColour.G / 65536.0 * param->doubles.imageAdjustments.mainLightIntensity * param->doubles.imageAdjustments.directLight;
						newColour.B += shadowOutputTemp.B * param->effectColours.mainLightColour.B / 65536.0 * param->doubles.imageAdjustments.mainLightIntensity * param->doubles.imageAdjustments.directLight;
					}
				}

				if (i > 0)
				{
					if (Lights[i - 1].enabled)
					{
						CVector3 d = Lights[i - 1].position - pointTemp;
						double distance = d.Length();
						double distance2 = distance * distance;
						CVector3 lightVectorTemp = d;
						lightVectorTemp.Normalize();
						double light = AuxShadow(param, calcParam, wsp_perspTemp, dist_threshTemp, distance, pointTemp, lightVectorTemp, param->penetratingLights);
						double intensity = Lights[i - 1].intensity * 100.0;
						newColour.R += light * Lights[i - 1].colour.R / 65536.0 / distance2 * intensity;
						newColour.G += light * Lights[i - 1].colour.G / 65536.0 / distance2 * intensity;
						newColour.B += light * Lights[i - 1].colour.B / 65536.0 / distance2 * intensity;
					}
				}

			}

			if(param->global_ilumination && !param->fastGlobalIllumination)
			{
				sShaderOutput AO = AmbientOcclusion(param, calcParam, pointTemp, wsp_perspTemp, dist_threshTemp, last_distance, vectorsAround, vectorsCount);
				newColour.R += AO.R * param->doubles.imageAdjustments.globalIlum;
				newColour.G += AO.G * param->doubles.imageAdjustments.globalIlum;
				newColour.B += AO.B * param->doubles.imageAdjustments.globalIlum;
			}

			if(opacity > 1.0) opacity = 1.0;
			{
				iterFog.R = iterFog.R * (1.0 - opacity) + newColour.R * opacity * fogColR/65536.0;
				iterFog.G = iterFog.G * (1.0 - opacity) + newColour.G * opacity * fogColG/65536.0;
				iterFog.B = iterFog.B * (1.0 - opacity) + newColour.B * opacity * fogColB/65536.0;
			}

			density+=opacity;

			//printf("iF.R=%g, dens=%g\n", iterFog.R, density);

			if(density>1.0) density = 1.0;
		}
	}

	iterFog.R *= 6553.0;
	iterFog.G *= 6553.0;
	iterFog.B *= 6553.0;
	if(iterFog.R > 65535) iterFog.R = 65535;
	if(iterFog.G > 65535) iterFog.G = 65535;
	if(iterFog.B > 65535) iterFog.B = 65535;

	*densityOut = density;

	//printf("R: %f, G: %f, B: %f\n", iterFog.R, iterFog.G, iterFog.B);

	return iterFog;
}

/*
void RenderBuddhabrot(sParamRender *fractParam)
{
	isPostRendering = true;

	if (isBuddhabrot)
	{
		delete buddhabrotImg;
	}
	buddhabrotImg = new sRGB[IMAGE_WIDTH * IMAGE_HEIGHT];
	isBuddhabrot = true;

	sRGB black = { 0, 0, 0 };
	for (int i = 0; i < IMAGE_WIDTH * IMAGE_HEIGHT; i++)
	{
		buddhabrotImg[i] = black;
	}

	double last_time = clock() / CLOCKS_PER_SEC;

	CVector3 point3D1, point3D2;

	CRotationMatrix mRot;
	mRot.RotateX(-fractParam->beta);
	mRot.RotateZ(-fractParam->alfa);

	double aspectRatio = (double) IMAGE_WIDTH / IMAGE_HEIGHT;

	sFractal calcParam;

	sFractal_ret calcRet;
	calcParam.mode = normal;

	CopyParams(fractParam, &calcParam);

	guint64 buddhCounter = 0;

	int i = 0;
	while (isPostRendering)
	{
		i++;
		CVector3 point;
		point.x = fractParam->vp.x + 0.5 * fractParam->zoom * (Random(2000000) / 1000000.0 - 1.0);
		point.y = fractParam->vp.y + 0.5 * fractParam->zoom * (Random(2000000) / 1000000.0 - 1.0);
		point.z = fractParam->vp.z + 0.5 * fractParam->zoom * (Random(2000000) / 1000000.0 - 1.0);

		calcParam.point = point;
		ComputeIterations(calcParam, calcRet);

		//printf("x=%f,y=%f,z=%f, maxL=%d\n", point.x, point.y, point.z, calcRet.L);

		//if (calcRet.L < fractParam->N+10)
		//{
		for (int L = 1; L < calcRet.L; L++)
		{
			point3D1 = buddhabrot[L].point - fractParam->vp;
			//printf("B1: x=%g,y=%g,z=%g\n", point3D1.x, point3D1.y, point3D1.z);
			point3D2 = mRot.RotateVector(point3D1);
			double y2 = point3D2.y;
			double y = y2 / fractParam->zoom;
			double wsp_persp = 1.0 + y * fractParam->persp;
			double x2 = point3D2.x / wsp_persp;
			double z2 = point3D2.z / wsp_persp;
			double x = (x2 / (fractParam->zoom * aspectRatio) + 0.5) * IMAGE_WIDTH;
			double z = (z2 / fractParam->zoom + 0.5) * IMAGE_HEIGHT;
			int intX = x;
			int intY = z;
			//printf("B2: x=%g,y=%g,z=%g\n", x, y, z);
			if (intX >= 0 && intX < IMAGE_WIDTH && intY >= 0 && intY < IMAGE_HEIGHT)
			{
				if (y < complexImage[intX + intY * IMAGE_WIDTH].zBuffer)
				{
					sRGB oldPixel = buddhabrotImg[intX + intY * IMAGE_WIDTH];
					int R = oldPixel.R + calcRet.L * 2;
					if (R > 65535) R = 65535;
					int G = oldPixel.G + L * 4;
					if (G > 65535) G = 65535;
					int B = oldPixel.B + fractParam->N;
					if (B > 65535) B = 65535;
					sRGB pixel = { R, G, B };
					buddhabrotImg[intX + intY * IMAGE_WIDTH] = pixel;
					buddhCounter++;

				}
			}
			//}
		}
		double time = clock() / CLOCKS_PER_SEC;
		if (time - last_time > 5)
		{
			buddhabrotAutoBright = 10000.0 * (double) IMAGE_WIDTH * IMAGE_HEIGHT / fractParam->N / buddhCounter;
			char progressText[1000];
			last_time = clock() / CLOCKS_PER_SEC;
			double done = (double) i;
			sprintf(progressText, "Rendering Buddhabrot. Done %.3g points", done);
			gtk_progress_bar_set_text(GTK_PROGRESS_BAR(Interface.progressBar), progressText);

			CompileImage();
			Bitmap32to8(complexImage, rgbbuf, rgbbuf2);

			RedrawImage(darea, rgbbuf);

			while (gtk_events_pending())
				gtk_main_iteration();
		}
	}
}
*/
