/*
 * fractparams.h
 *
 *  Created on: 2010-05-03
 *      Author: krzysztof
 */

#ifndef FRACTPARAMS_H_
#define FRACTPARAMS_H_

#include "fractal.h"
#include "texture.hpp"

enum dataMode
{
	soundNone = 0, soundEnvelope = 1, soundA = 2, soundB = 3, soundC = 4, soundD = 5
};

struct sParamRenderD
{
	CVector3 vp; //view point
	CVector3 auxLightPre1;
	CVector3 auxLightPre2;
	CVector3 auxLightPre3;
	CVector3 auxLightPre4;
	CVector3 auxLightRandomCenter;

	double zoom; //zoom
	double min_y; //range of depth;
	double max_y;
	double DE_factor; //factor for distance estimation steps
	double dist_thresh; //distance treshold
	double resolution; //resolution of image in fractal coordinates
	double persp; //perspective factor
	double quality; //DE threshold factor
	double smoothness;
	double alfa; //rotation of fractal
	double beta; //
	double gamma;
	double DOFFocus;
	double DOFRadius;
	double mainLightAlfa;
	double mainLightBeta;
	double auxLightIntensity;
	double auxLightMaxDist;
	double auxLightDistributionRadius;
	double auxLightVisibility;
	double auxLightPre1intensity;
	double auxLightPre2intensity;
	double auxLightPre3intensity;
	double auxLightPre4intensity;
	double soundFPS;
	double stereoEyeDistance;
	double viewDistanceMin;
	double viewDistanceMax;

	sImageAdjustments imageAdjustments;
};

struct sParamRender
{
	sParamRenderD doubles;

	sFractal fractal;
	int image_width; //image width
	int image_height; //image height
	int globalIlumQuality; //ambient occlusion quality
	int coloring_seed; //colouring random seed
	int auxLightRandomSeed;
	int auxLightNumber;
	int SSAOQuality;
	int startFrame;
	int endFrame;
	int framesPerKeyframe;
	int imageFormat;
	int soundBand1Min;
	int soundBand1Max;
	int soundBand2Min;
	int soundBand2Max;
	int soundBand3Min;
	int soundBand3Max;
	int soundBand4Min;
	int soundBand4Max;
	enumPerspectiveType perspectiveType;

	bool shadow; //enable shadows
	bool global_ilumination; //enable global ilumination
	bool fastGlobalIllumination; //enable fake global ilumination
	bool slowShading; //enable fake gradient calculation for shading
	bool textured_background; //enable testured background
	bool recordMode; //path recording mode
	bool continueRecord; //continue recording mode
	bool playMode; //play mode
	bool animMode; //animation mode
	bool SSAOEnabled;
	bool DOFEnabled;
	bool auxLightPre1Enabled;
	bool auxLightPre2Enabled;
	bool auxLightPre3Enabled;
	bool auxLightPre4Enabled;
	bool soundEnabled;
	bool stereoEnabled;
	bool quiet;
	sImageSwitches imageSwitches;

	sRGB background_color1; //background colour
	sRGB background_color2;
	sRGB auxLightPre1Colour;
	sRGB auxLightPre2Colour;
	sRGB auxLightPre3Colour;
	sRGB auxLightPre4Colour;
	sEffectColours effectColours;

	sRGB palette[256];

	char file_destination[1000];
	char file_envmap[1000];
	char file_background[1000];
	char file_lightmap[1000];
	char file_path[1000];
	char file_keyframes[1000];
	char file_sound[1000];

	cTexture *backgroundTexture;
	cTexture *envmapTexture;
	cTexture *lightmapTexture;
	std::vector<enumFractalFormula> formulaSequence;
	std::vector<double> hybridPowerSequence;
};

#endif /* FRACTPARAMS_H_ */
