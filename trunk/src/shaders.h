/*
 * shaders.h
 *
 *  Created on: 2010-04-24
 *      Author: Krzysztof Marczak
 */

#ifndef SHADERS_H_
#define SHADERS_H_

#include "Render3D.h"

struct sShaderOutput
{
	double R;
	double G;
	double B;
};

struct sLight
{
	CVector3 position;
	sRGB colour;
	double intensity;
	bool enabled;
};

struct sShaderInputData
{
	sParamRender *param;
	sFractal *calcParam;
	CVector3 point;
	CVector3 viewVector;
	CVector3 normal;
	CVector3 lightVect;
	double dist_thresh;
	double lastDist;
	sVectorsAround *vectorsAround;
	int vectorsCount;
	cTexture *envMappingTexture;
};

extern sLight *Lights;
extern int lightsPlaced;

sShaderOutput ObjectShader(sShaderInputData input, sShaderOutput *surfaceColour);

sShaderOutput MainShadow(sShaderInputData &input);
sShaderOutput AmbientOcclusion(sShaderInputData &input);
sShaderOutput FastAmbientOcclusion(sShaderInputData &input);
sShaderOutput FastAmbientOcclusion2(sShaderInputData &input);
CVector3 CalculateNormals(sShaderInputData input);
sShaderOutput MainShading(sShaderInputData &input);
sShaderOutput MainSpecular(sShaderInputData &input);
sShaderOutput EnvMapping(sShaderInputData &input);
sShaderOutput SurfaceColour(sShaderInputData &input);
sShaderOutput TexturedBackground(sParamRender *param, CVector3 viewVector);
sShaderOutput LightShading(sShaderInputData &input, sLight light, int number, sShaderOutput *specular);
sShaderOutput AuxLightsShader(sShaderInputData &input, sShaderOutput *specularOut);
double AuxShadow(sShaderInputData &input, double distance, CVector3 lightVector);
sShaderOutput FakeLights(sShaderInputData &input, sShaderOutput *fakeSpec);

sShaderOutput VolumetricLight(sParamRender *param, sFractal *calcParam, CVector3 point, double yStart, double min_y, double last_distance, double zoom, CVector3 lightVector);
sShaderOutput VolumetricFog(sParamRender *param, int buffCount, double *distanceBuff, double *stepBuff, double *densityOut);
void PlaceRandomLights(sParamRender *fractParams, bool onlyPredefined);
void PostRenderingLights(cImage *image, sParamRender *fractParam);
double IterOpacity(double step, double iters, double maxN, double trim, double opacitySp);
sShaderOutput IterationFog(sParamRender *param, sFractal *calcParam, CVector3 point, double yStart, double min_y, double last_distance, double zoom,
		CVector3 lightVector, bool found, double *densityOut, sVectorsAround *vectorsAround, int vectorsCount, sRGB16 oldPixel);

sRGB IndexToColour(int index, sRGB *palette);
#endif /* SHADERS_H_ */
