/*
 * fractal.h
 *
 *  Created on: 2010-01-23
 *      Author: krzysztof marczak
 */

#ifndef FRACTAL_H_
#define FRACTAL_H_

#include "common_math.h"
#include "algebra.hpp"

enum enumFractalFormula
{
	none = 0, trig_DE = 1, trig = 2, fast_trig = 3, hypercomplex = 4, quaternion = 5, minus_fast_trig = 6, menger_sponge = 7, tglad = 8, kaleidoscopic = 10, xenodreambuie = 11, hybrid = 12
};

enum enumCalculationMode
{
	normal = 0, colouring = 1, fake_AO = 2, deltaDE1 = 3, deltaDE2 = 4
};

struct sFractal
{
	double amin;
	double amax;
	double bmin;
	double bmax;
	double cmin;
	double cmax;
	bool limits_enabled;
	int N;
	int minN;
	bool juliaMode;
	CVector3 julia;
	double foldingLimit;
	double foldingValue;
	double foldingSphericalMin;
	double foldingSphericalFixed;
	bool tgladFoldingMode;
	bool sphericalFoldingMode;
	bool IFSFoldingMode;
	bool iterThresh;
	double DE_threshold;
	double power;
	double ca;
	double cb;
	double cc;
	int mode;
	enumFractalFormula formula;
	bool analitycDE;
	int required_N;
	CVector3 point;
	CVector3 *IFSDirection;
	double *IFSDistance;
	bool *IFSEnabled;
	double *IFSAlfa;
	double *IFSBeta;
	double *IFSGamma;
	double *IFSIntensity;
	double IFSRotationAlfa;
	double IFSRotationBeta;
	double IFSRotationGamma;
	bool IFSAbsX;
	bool IFSAbsY;
	bool IFSAbsZ;
	double IFSScale;
	CVector3 IFSOffset;
	int IFSfoldingCount;
	CRotationMatrix *IFSRot;
	CRotationMatrix IFSMainRot;
	double hybridPower1;
	double hybridPower2;
	double hybridPower3;
	double hybridPower4;
	double hybridPower5;
	int hybridIters1;
	int hybridIters2;
	int hybridIters3;
	int hybridIters4;
	int hybridIters5;
	enumFractalFormula hybridFormula1;
	enumFractalFormula hybridFormula2;
	enumFractalFormula hybridFormula3;
	enumFractalFormula hybridFormula4;
	enumFractalFormula hybridFormula5;
	enumFractalFormula *formulaSequence;
	double *hybridPowerSequence;
	bool hybridCyclic;

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

	double mandelboxColorFactorX;
	double mandelboxColorFactorY;
	double mandelboxColorFactorZ;
	double mandelboxColorFactorR;
	double mandelboxColorFactorSp1;
	double mandelboxColorFactorSp2;

	double mandelboxScale;
	double mandelboxFoldingLimit;
	double mandelboxFoldingValue;
	double mandelboxFoldingSphericalMin;
	double mandelboxFoldingSphericalFixed;

	bool mandelboxRotationsEnabled;
};

struct sFractal_ret
{
	double distance;
	int L;
	double fake_ao;
	double orbit;
	bool max_iter;
	double r1;
	double r2;
	double r3;
	double x_end;
	double y_end;
	double z_end;
	double colour;
};

struct sBuddhabrot
{
	CVector3 point;
};

extern sBuddhabrot buddhabrot[1000];

int ComputeIterations(sFractal &par, sFractal_ret &retVal);
double CalculateDistance(sFractal &params, sFractal_ret &ret);

#endif /* FRACTAL_H_ */
