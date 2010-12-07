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

const int IFS_VECTOR_COUNT = 9;
const int HYBRID_COUNT = 5;
const int MANDELBOX_FOLDS = 2;

enum enumFractalFormula
{
	none = 0,
	trig_DE = 1,
	trig = 2,
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
	mandelbulb4 = 15
};

enum enumCalculationMode
{
	normal = 0, colouring = 1, fake_AO = 2, deltaDE1 = 3, deltaDE2 = 4
};

struct sFractalIFS
{
	CVector3 offset;
	double scale;
	double rotationAlfa;
	double rotationBeta;
	double rotationGamma;
	bool absX, absY, absZ;

	bool foldingMode; // Kaleidoscopic IFS folding mode
	int foldingCount;

	CRotationMatrix mainRot;
	bool enabled[IFS_VECTOR_COUNT];
	CVector3 direction[IFS_VECTOR_COUNT];
	CRotationMatrix rot[IFS_VECTOR_COUNT];

	double distance[IFS_VECTOR_COUNT];
	double alfa[IFS_VECTOR_COUNT];
	double beta[IFS_VECTOR_COUNT];
	double gamma[IFS_VECTOR_COUNT];
	double intensity[IFS_VECTOR_COUNT];
};

struct sFractalMandelbox
{
	bool rotationsEnabled;
	double rotationMain[3];
	double rotation[MANDELBOX_FOLDS][3][3];

	CRotationMatrix mainRot;
	CRotationMatrix rot[MANDELBOX_FOLDS][3];
	CRotationMatrix rotinv[MANDELBOX_FOLDS][3];

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
};

struct sFractal
{
	int N;		  // maximum number of iterations
	int minN;	  // minimum number of iterations

	bool limits_enabled; // enable limits (intersections)
	double amin;  //fractal limits
	double amax;
	double bmin;
	double bmax;
	double cmin;
	double cmax;

	bool juliaMode;				// Julia mode
	bool tgladFoldingMode;		// Tglad's folding mode
	bool sphericalFoldingMode;  // spherical folding mode
	bool interiorMode;
	bool hybridCyclic;

	CVector3 julia; // Julia constant
	double constantFactor;

	double foldingLimit; //paramters of TGlad's folding
	double foldingValue;
	double foldingSphericalMin;
	double foldingSphericalFixed;

	bool iterThresh;	 //maxiter threshold mode
	bool analitycDE;	 //analytic DE mode
	double DE_threshold;
	double power;		 //power of fractal formula
	enumFractalFormula formula;

	double hybridPower[HYBRID_COUNT];
	int hybridIters[HYBRID_COUNT];
	enumFractalFormula hybridFormula[HYBRID_COUNT];

	std::vector<enumFractalFormula> formulaSequence;
	std::vector<double> hybridPowerSequence;

	sFractalIFS IFS;
	sFractalMandelbox mandelbox;
};

struct sBuddhabrot
{
	CVector3 point;
};

template <int Mode>
double Compute(CVector3 z, const sFractal &par, int *iter_count = NULL);

double CalculateDistance(CVector3 point, const sFractal &par, bool *max_iter = NULL);

extern sBuddhabrot buddhabrot[1000];

#endif /* FRACTAL_H_ */
