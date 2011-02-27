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

extern sLight *Lights;
extern int lightsPlaced;

sShaderOutput MainShadow(sParamRender *param, sFractal *calcParam, CVector3 point, CVector3 lightVect, double wsp_persp, double dist_thresh);
sShaderOutput AmbientOcclusion(sParamRender *param, sFractal *calcParam, CVector3 point, double wsp_persp, double dist_thresh, double last_distance, sVectorsAround *vectorsAround,
		int vectorsCount, CVector3 normal);
CVector3 CalculateNormals(sParamRender *param, sFractal *calcParam, CVector3 point, double wsp_persp, double dist_thresh, double last_distance);
sShaderOutput MainShading(CVector3 normal, CVector3 lightVector);
sShaderOutput MainSpecular(CVector3 normal, CVector3 lightVector, CVector3 viewVector);
sShaderOutput EnvMapping(CVector3 normal, CVector3 viewVector, cTexture *texture);
sShaderOutput LightShading(sParamRender *fractParams, sFractal *calcParam, CVector3 point, CVector3 normal, CVector3 viewVector, sLight light, double wsp_persp, double dist_thresh,
		int number, sShaderOutput *outSpecular, bool accurate);
void PlaceRandomLights(sParamRender *fractParams, bool onlyPredefined);
void PostRenderingLights(cImage *image, sParamRender *fractParam);
void RenderBuddhabrot(sParamRender *fractParam);
#endif /* SHADERS_H_ */
