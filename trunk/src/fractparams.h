/*
 * fractparams.h
 *
 *  Created on: 2010-05-03
 *      Author: krzysztof
 */

#ifndef FRACTPARAMS_H_
#define FRACTPARAMS_H_

#include "fractal.h"
#include "image.h"
#include "algebra.hpp"
#include "cimage.hpp"

enum dataMode
{
	soundNone = 0, soundEnvelope = 1, soundA = 2, soundB = 3, soundC = 4, soundD = 5
};

struct sAddData
{
	dataMode mode;
	double amp;
	bool active;
};

struct sParamRenderD
{
	CVector3 vp; //view point
	CVector3 julia; //Julia constant
	CVector3 auxLightPre1;
	CVector3 auxLightPre2;
	CVector3 auxLightPre3;
	CVector3 auxLightPre4;
	CVector3 IFSOffset;

	double amin; //fractal limits
	double amax;
	double bmin;
	double bmax;
	double cmin;
	double cmax;
	double zoom; //zoom
	double min_y; //range of depth;
	double max_y;
	double power; //power of fractal formula
	double foldingLimit; //parameters of Tg;ad's folding
	double foldingValue;
	double foldingSphericalMin; //parameters of spherical folding
	double foldingSphericalFixed;
	double DE_factor; //factor for distance estimation steps
	double dist_thresh; //distance treshold
	double resolution; //resolution of image in fractal coordinates
	double persp; //perspective factor
	double quality; //DE threshold factor
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
	double IFSRotationAlfa;
	double IFSRotationBeta;
	double IFSRotationGamma;
	double IFSScale;
	double soundFPS;
	double hybridPower1;
	double hybridPower2;
	double hybridPower3;
	double hybridPower4;
	double hybridPower5;
	double stereoEyeDistance;
	double mandelboxScale;
	double mandelboxColorFactorX;
	double mandelboxColorFactorY;
	double mandelboxColorFactorZ;
	double mandelboxColorFactorR;
	double mandelboxColorFactorSp1;
	double mandelboxColorFactorSp2;
	double mandelboxRotationMainAlfa;
	double mandelboxRotationMainBeta;
	double mandelboxRotationMainGamma;
	double mandelboxRotationX1Alfa;
	double mandelboxRotationX1Beta;
	double mandelboxRotationX1Gamma;
	double mandelboxRotationX2Alfa;
	double mandelboxRotationX2Beta;
	double mandelboxRotationX2Gamma;
	double mandelboxRotationY1Alfa;
	double mandelboxRotationY1Beta;
	double mandelboxRotationY1Gamma;
	double mandelboxRotationY2Alfa;
	double mandelboxRotationY2Beta;
	double mandelboxRotationY2Gamma;
	double mandelboxRotationZ1Alfa;
	double mandelboxRotationZ1Beta;
	double mandelboxRotationZ1Gamma;
	double mandelboxRotationZ2Alfa;
	double mandelboxRotationZ2Beta;
	double mandelboxRotationZ2Gamma;
	double mandelboxFoldingLimit;
	double mandelboxFoldingValue;
	double mandelboxFoldingSphericalMin;
	double mandelboxFoldingSphericalFixed;
	double viewDistanceMin;
	double viewDistanceMax;
	double fractalConstantFactor;

	sImageAdjustments imageAdjustments;
};

struct sParamRender
{
	sParamRenderD doubles;

	double *IFSDistance;
	double *IFSAlfa;
	double *IFSBeta;
	double *IFSGamma;
	double *IFSIntensity;

	int N; //maximum number of iterations
	int minN; //minimum number of iterations
	int image_width; //image width
	int image_height; //image height
	int globalIlumQuality; //ambient occlusion quality
	int coloring_seed; //colouring random seed
	int auxLightRandomSeed;
	int auxLightNumber;
	int SSAOQuality;
	int IFSfoldingCount;
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
	int hybridIters1;
	int hybridIters2;
	int hybridIters3;
	int hybridIters4;
	int hybridIters5;

	bool iterThresh; //maxiter threshord mode
	bool analitycDE; //analytic DE mode
	bool juliaMode; //Julia mode
	bool tgladFoldingMode; //Tglad's folding mode
	bool sphericalFoldingMode; //spherical folding mode
	bool IFSFoldingMode; //Kaleidoskopic IFS folding mode
	bool shadow; //enable shadows
	bool global_ilumination; //enable global ilumination
	bool fastGlobalIllumination; //enable fake global ilumination
	bool slowShading; //enable fake gradient calculation for shading
	bool limits_enabled; //enable limits (intersections)
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
	bool IFSAbsX;
	bool IFSAbsY;
	bool IFSAbsZ;
	bool soundEnabled;
	bool hybridCyclic;
	bool fishEye;
	bool stereoEnabled;
	bool quiet;
	bool mandelboxRotationsEnabled;
	bool interiorMode;
	sImageSwitches imageSwitches;

	enumFractalFormula formula; //type of fractal formula
	enumFractalFormula hybridFormula1;
	enumFractalFormula hybridFormula2;
	enumFractalFormula hybridFormula3;
	enumFractalFormula hybridFormula4;
	enumFractalFormula hybridFormula5;

	sRGB background_color1; //background colour
	sRGB background_color2;
	sRGB auxLightPre1Colour;
	sRGB auxLightPre2Colour;
	sRGB auxLightPre3Colour;
	sRGB auxLightPre4Colour;
	sEffectColours effectColours;

	CRotationMatrix IFSMainRot;

	CRotationMatrix mandelboxMainRot;
	CRotationMatrix mandelboxRot1X;
	CRotationMatrix mandelboxRot2X;
	CRotationMatrix mandelboxRot1Y;
	CRotationMatrix mandelboxRot2Y;
	CRotationMatrix mandelboxRot1Z;
	CRotationMatrix mandelboxRot2Z;
	CRotationMatrix mandelboxRot1Xinv;
	CRotationMatrix mandelboxRot2Xinv;
	CRotationMatrix mandelboxRot1Yinv;
	CRotationMatrix mandelboxRot2Yinv;
	CRotationMatrix mandelboxRot1Zinv;
	CRotationMatrix mandelboxRot2Zinv;

	sRGB *palette;

	char file_destination[1000];
	char file_envmap[1000];
	char file_background[1000];
	char file_lightmap[1000];
	char file_path[1000];
	char file_keyframes[1000];
	char file_sound[1000];

	bool *IFSEnabled;
	cTexture *backgroundTexture;
	cTexture *envmapTexture;
	cTexture *lightmapTexture;
	CVector3 *IFSDirection;
	CRotationMatrix *IFSRot;
	enumFractalFormula *formulaSequence;
	double *hybridPowerSequence;
};

struct sParamSpecial
{
	sAddData vpX;
	sAddData vpY;
	sAddData vpZ;
	sAddData juliaX;
	sAddData juliaY;
	sAddData juliaZ;
	sAddData auxLightPre1X;
	sAddData auxLightPre1Y;
	sAddData auxLightPre1Z;
	sAddData auxLightPre2X;
	sAddData auxLightPre2Y;
	sAddData auxLightPre2Z;
	sAddData auxLightPre3X;
	sAddData auxLightPre3Y;
	sAddData auxLightPre3Z;
	sAddData auxLightPre4X;
	sAddData auxLightPre4Y;
	sAddData auxLightPre4Z;
	sAddData IFSOffsetX;
	sAddData IFSOffsetY;
	sAddData IFSOffsetZ;
	sAddData amin; //fractal limits
	sAddData amax;
	sAddData bmin;
	sAddData bmax;
	sAddData cmin;
	sAddData cmax;
	sAddData zoom; //zoom
	sAddData min_y; //range of depth;
	sAddData max_y;
	sAddData power; //power of fractal formula
	sAddData foldingLimit; //parameters of Tg;ad's folding
	sAddData foldingValue;
	sAddData foldingSphericalMin; //parameters of spherical folding
	sAddData foldingSphericalFixed;
	sAddData DE_factor; //factor for distance estimation steps
	sAddData dist_thresh; //distance treshold
	sAddData resolution; //resolution of image in fractal coordinates
	sAddData brightness; //image brightness
	sAddData ambient; //simple ambient light
	sAddData reflect; //reflection factor
	sAddData directLight; //direct light intensity
	sAddData globalIlum; //intensity of ambient occlusion
	sAddData shading; //intensity of shading
	sAddData imageGamma; //image gamma
	sAddData specular; //intensity of specularity effect
	sAddData glow_intensity; //intensity of glow effect
	sAddData persp; //perspective factor
	sAddData quality; //DE threshold factor
	sAddData alfa; //rotation of fractal
	sAddData beta; //
	sAddData gamma;
	sAddData coloring_speed; //colour change frequency
	sAddData fogVisibility;
	sAddData fogVisibilityFront;
	sAddData DOFFocus;
	sAddData DOFRadius;
	sAddData mainLightAlfa;
	sAddData mainLightBeta;
	sAddData mainLightIntensity;
	sAddData auxLightIntensity;
	sAddData auxLightMaxDist;
	sAddData auxLightDistributionRadius;
	sAddData auxLightVisibility;
	sAddData auxLightPre1intensity;
	sAddData auxLightPre2intensity;
	sAddData auxLightPre3intensity;
	sAddData auxLightPre4intensity;
	sAddData IFSRotationAlfa;
	sAddData IFSRotationBeta;
	sAddData IFSRotationGamma;
	sAddData IFSScale;
	sAddData paletteOffset;
	sAddData hybridPower1;
	sAddData hybridPower2;
	sAddData hybridPower3;
	sAddData hybridPower4;
	sAddData hybridPower5;
	sAddData stereoEyeDistance;
	sAddData mandelboxScale;
	sAddData mandelboxColorFactorX;
	sAddData mandelboxColorFactorY;
	sAddData mandelboxColorFactorZ;
	sAddData mandelboxColorFactorR;
	sAddData mandelboxColorFactorSp1;
	sAddData mandelboxColorFactorSp2;
	sAddData mandelboxRotationMainAlfa;
	sAddData mandelboxRotationMainBeta;
	sAddData mandelboxRotationMainGamma;
	sAddData mandelboxRotationX1Alfa;
	sAddData mandelboxRotationX1Beta;
	sAddData mandelboxRotationX1Gamma;
	sAddData mandelboxRotationY1Alfa;
	sAddData mandelboxRotationY1Beta;
	sAddData mandelboxRotationY1Gamma;
	sAddData mandelboxRotationZ1Alfa;
	sAddData mandelboxRotationZ1Beta;
	sAddData mandelboxRotationZ1Gamma;
	sAddData mandelboxRotationX2Alfa;
	sAddData mandelboxRotationX2Beta;
	sAddData mandelboxRotationX2Gamma;
	sAddData mandelboxRotationY2Alfa;
	sAddData mandelboxRotationY2Beta;
	sAddData mandelboxRotationY2Gamma;
	sAddData mandelboxRotationZ2Alfa;
	sAddData mandelboxRotationZ2Beta;
	sAddData mandelboxRotationZ2Gamma;
	sAddData mandelboxFoldingLimit;
	sAddData mandelboxFoldingValue;
	sAddData mandelboxFoldingSphericalMin;
	sAddData mandelboxFoldingSphericalFixed;
	sAddData viewDistanceMin;
	sAddData viewDistanceMax;
	sAddData fractalConstantFactor;
};

#endif /* FRACTPARAMS_H_ */
