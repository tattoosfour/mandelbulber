/*
 * fractal.h
 *
 *  Created on: 2010-01-23
 *      Author: krzysztof marczak
 */

#ifndef FRACTAL_H_
#define FRACTAL_H_

#include <vector>
#include "common_math.h"
#include "fractparams.h"
#include <stddef.h>

const int IFS_VECTOR_COUNT = 9;
const int HYBRID_COUNT = 5;
const int MANDELBOX_FOLDS = 2;

enum enumFractalFormula
{
	none = 0,
	trig_DE = 1,
	trig_optim = 2,
	fast_trig = 3,
	hypercomplex = 4,
	quaternion = 5,
	minus_fast_trig = 6,
	menger_sponge = 7,
	tglad = 8,
	kaleidoscopic = 10,
	xenodreambuie = 11,
	hybrid = 12,
	mandelbulb2 = 13,
	mandelbulb3 = 14,
	mandelbulb4 = 15,
	foldingIntPow2 = 16,
	smoothMandelbox = 17,
	mandelboxVaryScale4D = 18,
	aexion = 19,
	benesi = 20,
	bristorbrot = 21
};

enum enumCalculationMode
{
	normal = 0, colouring = 1, fake_AO = 2, deltaDE1 = 3, deltaDE2 = 4
};

struct sFractalIFSD
{
	double rotationGamma;
	double rotationAlfa;
	double rotationBeta;
	double scale;
	double distance[IFS_VECTOR_COUNT];
	double alfa[IFS_VECTOR_COUNT];
	double beta[IFS_VECTOR_COUNT];
	double gamma[IFS_VECTOR_COUNT];
	double intensity[IFS_VECTOR_COUNT];
	CVector3 offset;
	CVector3 direction[IFS_VECTOR_COUNT];
};

struct sFractalIFS
{
	sFractalIFSD doubles;
	bool absX, absY, absZ;
	bool foldingMode; // Kaleidoscopic IFS folding mode
	bool enabled[IFS_VECTOR_COUNT];
	int foldingCount;
	CRotationMatrix mainRot;
	CRotationMatrix rot[IFS_VECTOR_COUNT];
};

struct sFractalMandelboxVary4D
{
	double fold;
	double minR;
	double scaleVary;
	double wadd;
	double rPower;
};

struct sFractalMandelboxD
{
	double rotationMain[3];
	double rotation[MANDELBOX_FOLDS][3][3];
	double colorFactorX;
	double colorFactorY;
	double colorFactorZ;
	double colorFactorR;
	double colorFactorSp1;
	double colorFactorSp2;
	double scale;
	double foldingLimit;
	double foldingValue;
	double foldingSphericalMin;
	double foldingSphericalFixed;
	double sharpness;
	CVector3 offset;
	sFractalMandelboxVary4D vary4D;
};

struct sFractalMandelbox
{
	sFractalMandelboxD doubles;
	bool rotationsEnabled;
	CRotationMatrix mainRot;
	CRotationMatrix rot[MANDELBOX_FOLDS][3];
	CRotationMatrix rotinv[MANDELBOX_FOLDS][3];
};


struct sFractalD
{
	double amin;  //fractal limits
	double amax;
	double bmin;
	double bmax;
	double cmin;
	double cmax;
	double constantFactor;
	double FoldingIntPowZfactor;
	double FoldingIntPowFoldFactor;
	double foldingLimit; //paramters of TGlad's folding
	double foldingValue;
	double foldingSphericalMin;
	double foldingSphericalFixed;
	double detailSize;
	double power;		 //power of fractal formula
	double cadd;
	double hybridPower[HYBRID_COUNT];
	CVector3 julia; // Julia constant
};

struct sFractal
{
	sFractalD doubles;

	int N;		  // maximum number of iterations
	int minN;	  // minimum number of iterations

	bool limits_enabled; // enable limits (intersections)
	bool iterThresh;	 //maxiter threshold mode
	bool analitycDE;	 //analytic DE mode
	bool juliaMode;				// Julia mode
	bool tgladFoldingMode;		// Tglad's folding mode
	bool sphericalFoldingMode;  // spherical folding mode
	bool interiorMode;
	bool hybridCyclic;
	bool dynamicDEcorrection;
	bool linearDEmode;
	bool constantDEThreshold;

	enumFractalFormula formula;

	int hybridIters[HYBRID_COUNT];
	enumFractalFormula hybridFormula[HYBRID_COUNT];

	std::vector<enumFractalFormula> formulaSequence;
	std::vector<double> hybridPowerSequence;

	sFractalIFS IFS;
	sFractalMandelbox mandelbox;
};

template <int Mode>

double Compute(CVector3 z, const sFractal &par, int *iter_count = NULL);
double CalculateDistance(CVector3 point, const sFractal &par, bool *max_iter = NULL);

#endif /* FRACTAL_H_ */
