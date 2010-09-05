/********************************************************
 /                   MANDELBULBER                        *
 /                                                       *
 / author: Krzysztof Marczak                             *
 / contact: buddhi1980@gmail.com                         *
 / licence: GNU GPL                                      *
 ********************************************************/

/*
 * fractal.cpp
 *
 *  Created on: 2010-01-23
 *      Author: krzysztof
 */

#include "fractal.h"
#include "math.h"
#include "image.h"
#include "Render3D.h"
#include "algebra.hpp"
#include "interface.h"

sBuddhabrot buddhabrot[1000];

//******************** COMPUTE ITERATIONS *********************
int ComputeIterations(sFractal &par, sFractal_ret &retVal)
{
	int L;
	double distance = 0;

	CVector3 z = par.point;
	double w = 0;
	double temp = 0;

	CVector3 dz(1.0, 0.0, 0.0);
	CVector3 one(1.0, 0.0, 0.0);
	double r_dz = 1;
	double ph_dz = 0;
	double th_dz = 0;
	double p = par.power; //mandelbulb power

	CVector3 constant;

	double fixedRadius = par.foldingSphericalFixed;
	double fR2 = fixedRadius * fixedRadius;
	double minRadius = par.foldingSphericalMin;
	double mR2 = minRadius * minRadius;
	double tglad_factor1 = fR2 / mR2;

	double tgladDE = par.power;

	enumFractalFormula actualFormula = par.formula;
	if (actualFormula == kaleidoscopic || actualFormula == menger_sponge) tgladDE = 1.0;
	double tgladColor = 1.0;

	if (par.juliaMode)
	{
		constant = par.julia;
	}
	else
	{
		constant = par.point;
	}

	bool hybridEnabled = false;
	if (actualFormula == hybrid) hybridEnabled = true;

	double r = z.Length();

	double min = 1e200;
	for (L = 0; L < par.N; L++)
	{
		//buddhabrot[L].point = z;

		if (hybridEnabled)
		{
			actualFormula = par.formulaSequence[L];
			p = par.hybridPowerSequence[L];
		}

		if (par.IFSFoldingMode)
		{
			if (par.IFSAbsX) z.x = fabs(z.x);
			if (par.IFSAbsY) z.y = fabs(z.y);
			if (par.IFSAbsZ) z.z = fabs(z.z);

			for (int i = 0; i < IFS_number_of_vectors; i++)
			{
				if (par.IFSEnabled[i])
				{
					z = par.IFSRot[i].RotateVector(z);
					double length = z.Dot(par.IFSDirection[i]);

					if (length < par.IFSDistance[i])
					{
						z -= par.IFSDirection[i] * 2.0 * (length - par.IFSDistance[i]);
					}

				}
			}

			z = par.IFSMainRot.RotateVector(z - par.IFSOffset) + par.IFSOffset;
			z *= par.IFSScale;
			z -= par.IFSOffset * (par.IFSScale - 1.0);

			r = z.Length();
		}

		if (par.tgladFoldingMode || actualFormula == tglad)
		{
			if (z.x > par.foldingLimit)
			{
				z.x = par.foldingValue - z.x;
				tgladColor *= 0.9;
			}
			else if (z.x < -par.foldingLimit)
			{
				z.x = -par.foldingValue - z.x;
				tgladColor *= 0.9;
			}
			if (z.y > par.foldingLimit)
			{
				z.y = par.foldingValue - z.y;
				tgladColor *= 0.9;
			}
			else if (z.y < -par.foldingLimit)
			{
				z.y = -par.foldingValue - z.y;
				tgladColor *= 0.9;
			}
			if (z.z > par.foldingLimit)
			{
				z.z = par.foldingValue - z.z;
				tgladColor *= 0.9;
			}
			else if (z.z < -par.foldingLimit)
			{
				z.z = -par.foldingValue - z.z;
				tgladColor *= 0.9;
			}
			r = z.Length();
		}

		if (par.sphericalFoldingMode)
		{
			double r2 = r * r;
			if (r2 < mR2)
			{
				z = z * tglad_factor1;
				tgladDE *= tglad_factor1;
				tgladColor += 1;
			}
			else if (r2 < fR2)
			{
				double tglad_factor2 = fR2 / r2;
				z = z * tglad_factor2;
				tgladDE *= tglad_factor2;
				tgladColor += 10;
			}
			r = z.Length();
		}

		switch (actualFormula)
		{
			case trig_DE:
			{
				double r1 = pow(r, p - 1);
				double r2 = r1 * r;
				double th = z.GetAlfa();
				double ph = -z.GetBeta();
				if (par.mode == 0)
				{
					double p_r1_rdz = p * r1 * r_dz;
					double ph_phdz = (p - 1.0) * ph + ph_dz;
					double th_thdz = (p - 1.0) * th + th_dz;
					CVector3 rot(th_thdz, ph_phdz);
					dz = rot * p_r1_rdz + one;
					r_dz = dz.Length();
					th_dz = dz.GetAlfa();
					ph_dz = -dz.GetBeta();
				}
				CVector3 rot(p * th, p * ph);
				z = rot * r2 + constant;
				r = z.Length();
				break;
			}
			case trig:
			{
				double rp = pow(r, p);
				double th = z.GetAlfa();
				double ph = -z.GetBeta();
				CVector3 rot(p * th, p * ph);
				z = rot * rp + constant;
				r = z.Length();
				break;
			}
			case xenodreambuie:
			{
				double rp = pow(r, p);
				double th = atan2(z.y, z.x);
				double ph = acos(z.z / r);
				if (ph > 0.5 * M_PI)
				{
					ph = M_PI - ph;
				}
				else if (ph < -0.5 * M_PI)
				{
					ph = -M_PI - ph;
				}
				z.x = rp * cos(th * p) * sin(ph * p);
				z.y = rp * sin(th * p) * sin(ph * p);
				z.z = rp * cos(ph * p);
				z = z + constant;
				r = z.Length();
				break;
			}
			case fast_trig:
			{
				double x2 = z.x * z.x;
				double y2 = z.y * z.y;
				double z2 = z.z * z.z;
				double temp = 1.0 - z2 / (x2 + y2);
				double newx = (x2 - y2) * temp;
				double newy = 2.0 * z.x * z.y * temp;
				double newz = -2.0 * z.z * sqrt(x2 + y2);
				z.x = newx + constant.x;
				z.y = newy + constant.y;
				z.z = newz + constant.z;
				r = z.Length();
				break;
			}
			case minus_fast_trig:
			{
				double x2 = z.x * z.x;
				double y2 = z.y * z.y;
				double z2 = z.z * z.z;
				double temp = 1.0 - z2 / (x2 + y2);
				double newx = (x2 - y2) * temp;
				double newy = 2.0 * z.x * z.y * temp;
				double newz = 2.0 * z.z * sqrt(x2 + y2);
				z.x = newx + constant.x;
				z.y = newy + constant.y;
				z.z = newz + constant.z;
				r = z.Length();
			}
			case hypercomplex:
			{
				CVector3 newz(z.x * z.x - z.y * z.y - z.z * z.z - w * w, 2.0 * z.x * z.y - 2.0 * w * z.z, 2.0 * z.x * z.z - 2.0 * z.y * w);
				double neww = 2.0 * z.x * w - 2.0 * z.y * z.z;
				z = newz + constant;
				w = neww;
				r = sqrt(z.x * z.x + z.y * z.y + z.z * z.z + w * w);
				break;
			}
			case quaternion:
			{
				CVector3 newz(z.x * z.x - z.y * z.y - z.z * z.z - w * w, 2.0 * z.x * z.y, 2.0 * z.x * z.z);
				double neww = 2.0 * z.x * w;
				z = newz + constant;
				w = neww;
				r = sqrt(z.x * z.x + z.y * z.y + z.z * z.z + w * w);
				break;
			}
			case menger_sponge:
			{
				z.x = fabs(z.x);
				z.y = fabs(z.y);
				z.z = fabs(z.z);
				if (z.x - z.y < 0)
				{
					temp = z.y;
					z.y = z.x;
					z.x = temp;
				}
				if (z.x - z.z < 0)
				{
					temp = z.z;
					z.z = z.x;
					z.x = temp;
				}
				if (z.y - z.z < 0)
				{
					temp = z.z;
					z.z = z.y;
					z.y = temp;
				}

				if (par.mode == colouring)
				{
					double length2 = z.Length();
					if (length2 < min) min = length2;
				}

				z *= 3.0;

				z.x -= 2.0;
				z.y -= 2.0;
				if (z.z > 1.0) z.z -= 2.0;
				r = z.Length();
				tgladDE *= 3.0;
				break;
			}
			case tglad:
			{
				double scale = p;
				double r2 = r * r;

				if (r2 < mR2)
				{
					z *= tglad_factor1;
					tgladDE *= tglad_factor1;
					tgladColor += 1;
				}
				else if (r2 < fR2)
				{
					double tglad_factor2 = fR2 / r2;
					z *= tglad_factor2;
					tgladDE *= tglad_factor2;
					tgladColor += 10;
				}
				z = z * scale + constant;
				tgladDE *= scale;

				r = z.Length();
				break;
			}
			case kaleidoscopic:
			{

				if (par.IFSAbsX) z.x = fabs(z.x);
				if (par.IFSAbsY) z.y = fabs(z.y);
				if (par.IFSAbsZ) z.z = fabs(z.z);

				for (int i = 0; i < IFS_number_of_vectors; i++)
				{
					if (par.IFSEnabled[i])
					{
						z = par.IFSRot[i].RotateVector(z);
						double length = z.Dot(par.IFSDirection[i]);

						if (length < par.IFSDistance[i])
						{
							z -= par.IFSDirection[i] * (2.0 * (length - par.IFSDistance[i]) * par.IFSIntensity[i]);
						}

					}
				}
				z = par.IFSMainRot.RotateVector(z - par.IFSOffset) + par.IFSOffset;

				if (par.mode == colouring)
				{
					double length2 = z.Length();
					if (length2 < min) min = length2;
				}

				z *= par.IFSScale;
				z -= par.IFSOffset * (par.IFSScale - 1.0);

				tgladDE *= par.IFSScale;
				r = z.Length();

				break;
			}
		}

		//************************** iteration terminate conditions *****************
		if (par.mode == deltaDE1)
		{
			if (r > 1e10)
			{
				retVal.r1 = r;
				break;
			}
		}
		else if (par.mode == deltaDE2)
		{
			if (L == par.required_N)
			{
				retVal.r1 = r;
				break;
			}
		}

		if (actualFormula == menger_sponge)
		{
			if (r > 1000)
			{
				retVal.distance = (r - 2.0) / tgladDE;
				break;
			}
		}
		else if (actualFormula == tglad)
		{
			if (r > 1024)
			{
				retVal.distance = r / fabs(tgladDE);
				break;
			}
		}
		else if (actualFormula == kaleidoscopic)
		{
			if (r > 1000)
			{
				retVal.distance = (r - 2.0) / tgladDE;
				break;
			}
		}
		else
		{
			if (par.mode == normal) //condition for all other trigonometric and hypercomplex fractals
			{
				if (r > 1e15)
				{
					retVal.distance = 0.5 * r * log(r) / r_dz;
					break;
				}
			}
			else if (par.mode == fake_AO) //mode 2
			{
				if (r < min) min = r;
				if (r > 1e15)
				{
					distance = min;
					break;
				}
			}
			else if (par.mode == deltaDE1)
			{
				if (r > 1e10)
				{
					retVal.r1 = r;
					break;
				}
			}
			else if (par.mode == deltaDE2)
			{
				if (L == par.required_N)
				{
					retVal.r1 = r;
					break;
				}
			}
			else if (par.mode == colouring) //mode 1
			{
				distance = z.Length();
				if (distance < min) min = distance;
				if (distance > 1e15)
				{
					distance = min;
					break;
				}
			}
		}
	}

	//************ return values *****************
	N_counter += L + 1;
	Loop_counter++;

	if (L < 64)
	{
		histogram[L]++;
	}
	else
	{
		histogram[63]++;
	}

	int L2 = 0;

	if (par.mode == normal || par.mode == deltaDE1 || par.mode == deltaDE2)
	{
		if (L == par.N)
		{
			L2 = 256;
			retVal.distance = 0;
		}
	}

	if (par.mode == fake_AO)
	{
		retVal.fake_ao = distance;
	}

	if (par.mode == colouring)
	{
		if (actualFormula == tglad)
		{
			retVal.colour = tgladColor * 100.0;
		}
		else if (actualFormula == kaleidoscopic || actualFormula == menger_sponge)
		{
			retVal.colour = min * 1000.0;
		}
		else
		{
			retVal.colour = distance * 5000.0;
		}

		if (par.formula == hybrid)
		{
			if (min > 100) min = 100;
			if (distance > 20) distance = 20;
			if (tgladColor > 1000) tgladColor = 1000;
			retVal.colour = distance * 5000.0 + tgladColor * 100.0 + min * 1000.0;
		}
	}

	retVal.x_end = z.x;
	retVal.y_end = z.y;
	retVal.z_end = z.z;
	retVal.r1 = r;
	retVal.L = L;
	return L2;

}

//******************* Calculate distance *******************8

double CalculateDistance(sFractal &params, sFractal_ret &ret)
{
	//limits


	if (params.limits_enabled)
	{
		bool limit = false;
		double distance_a = 0;
		double distance_b = 0;
		double distance_c = 0;

		if (params.point.x < params.amin)
		{
			distance_a = fabs(params.amin - params.point.x) + params.DE_threshold;
			limit = true;
		}
		if (params.point.x > params.amax)
		{
			distance_a = fabs(params.amax - params.point.x) + params.DE_threshold;
			limit = true;
		}

		if (params.point.y < params.bmin)
		{
			distance_a = fabs(params.bmin - params.point.y) + params.DE_threshold;
			limit = true;
		}
		if (params.point.y > params.bmax)
		{
			distance_b = fabs(params.bmax - params.point.y) + params.DE_threshold;
			limit = true;
		}

		if (params.point.z < params.cmin)
		{
			distance_c = fabs(params.cmin - params.point.z) + params.DE_threshold;
			limit = true;
		}
		if (params.point.z > params.cmax)
		{
			distance_c = fabs(params.cmax - params.point.z) + params.DE_threshold;
			limit = true;
		}

		if (limit)
		{
			ret.max_iter = false;
			ret.distance = dMax(dMax(distance_a, distance_b), distance_c);
			ret.L = 0;
			return ret.distance;
		}
	}

	if (params.analitycDE)
	{
		params.mode = normal;
		int retval = ComputeIterations(params, ret);
		if (retval == 256) ret.max_iter = true;
		else ret.max_iter = false;

		if (ret.L < params.minN && ret.distance < params.DE_threshold)
		{
			ret.distance = params.DE_threshold;
		}

		//ret.distance = dMin(ret.distance,0.0-params.c);
		if (ret.distance < 0) ret.distance = 0;

		return ret.distance;

	}
	else
	{
		double deltaDE = 1e-10;

		params.mode = deltaDE1;
		int retval = ComputeIterations(params, ret);
		double r = ret.r1;

		params.point.x += deltaDE;
		params.point.y += 0;
		params.point.z += 0;
		params.mode = deltaDE2;
		params.required_N = ret.L;
		ComputeIterations(params, ret);
		double r2 = ret.r1;
		double dr1 = fabs(r2 - r) / deltaDE;

		params.point.x -= deltaDE;
		params.point.y += deltaDE;
		params.point.z += 0;
		params.mode = deltaDE2;
		params.required_N = ret.L;
		ComputeIterations(params, ret);
		r2 = ret.r1;
		double dr2 = fabs(r2 - r) / deltaDE;

		params.point.x += 0;
		params.point.y -= deltaDE;
		params.point.z += deltaDE;
		params.mode = deltaDE2;
		params.required_N = ret.L;
		ComputeIterations(params, ret);
		r2 = ret.r1;
		double dr3 = fabs(r2 - r) / deltaDE;

		double dr = sqrt(dr1 * dr1 + dr2 * dr2 + dr3 * dr3);

		ret.distance = 0.5 * r * log(r) / dr;

		if (retval == 256)
		{
			ret.max_iter = true;
			ret.distance = 0;
		}
		else ret.max_iter = false;

		if (ret.L < params.minN && ret.distance < params.DE_threshold)
		{
			ret.distance = params.DE_threshold;
		}

		//ret.distance = dMin(ret.distance,0.0-params.c);
		if (ret.distance < 0) ret.distance = 0;

		return ret.distance;
	}
}
