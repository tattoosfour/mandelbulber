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

#include "Render3D.h"
#include "interface.h"

/**
 * Compute the fractal at the point, in one of the various modes
 *
 * Mode: normal: Returns distance
 *		 fake_ao: Returns minimum radius
 *		 colouring: Returns colour index
 *		 delta_DE1, delta_DE2: Returns radius
 */
template<int Mode>
double Compute(CVector3 z, const sFractal &par, int *iter_count)
{
	int L;
	double distance = 0;

	double w = 0;

	CVector3 dz(1.0, 0.0, 0.0);
	CVector3 one(1.0, 0.0, 0.0);
	double r_dz = 1;
	double ph_dz = 0;
	double th_dz = 0;
	double p = par.doubles.power; //mandelbulb power

	CVector3 constant;

	double fixedRadius = par.mandelbox.doubles.foldingSphericalFixed;
	double fR2 = fixedRadius * fixedRadius;
	double minRadius = par.mandelbox.doubles.foldingSphericalMin;
	double mR2 = minRadius * minRadius;
	double tglad_factor1 = fR2 / mR2;

	double tgladDE = 1.0;

	double scale = par.mandelbox.doubles.scale;

	enumFractalFormula actualFormula = par.formula;
	if (actualFormula == kaleidoscopic || actualFormula == menger_sponge) 
		tgladDE = 1.0;

	double tgladColor = 1.0;

	if (par.juliaMode)
	{
		constant = par.doubles.julia;
	}
	else
	{
		constant = z * par.doubles.constantFactor;
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
			int tempL = L;
			if(tempL > (int)par.formulaSequence.size()-1) tempL = (int)par.formulaSequence.size()-1;
			actualFormula = par.formulaSequence[tempL];
			p = par.hybridPowerSequence[tempL];
			scale = p;
		}

		if (par.IFS.foldingMode)
		{
			if (par.IFS.absX) z.x = fabs(z.x);
			if (par.IFS.absY) z.y = fabs(z.y);
			if (par.IFS.absZ) z.z = fabs(z.z);

			for (int i = 0; i < IFS_VECTOR_COUNT; i++)
			{
				if (par.IFS.enabled[i])
				{
					z = par.IFS.rot[i].RotateVector(z);
					double length = z.Dot(par.IFS.doubles.direction[i]);

					if (length < par.IFS.doubles.distance[i])
					{
						z -= par.IFS.doubles.direction[i] * 2.0 * (length - par.IFS.doubles.distance[i]);
					}

				}
			}

			z = par.IFS.mainRot.RotateVector(z - par.IFS.doubles.offset) + par.IFS.doubles.offset;
			z *= par.IFS.doubles.scale;
			z -= par.IFS.doubles.offset * (par.IFS.doubles.scale - 1.0);

			r = z.Length();
		}

		if (par.tgladFoldingMode)
		{
			if (z.x > par.doubles.foldingLimit)
			{
				z.x = par.doubles.foldingValue - z.x;
				tgladColor *= 0.9;
			}
			else if (z.x < -par.doubles.foldingLimit)
			{
				z.x = -par.doubles.foldingValue - z.x;
				tgladColor *= 0.9;
			}
			if (z.y > par.doubles.foldingLimit)
			{
				z.y = par.doubles.foldingValue - z.y;
				tgladColor *= 0.9;
			}
			else if (z.y < -par.doubles.foldingLimit)
			{
				z.y = -par.doubles.foldingValue - z.y;
				tgladColor *= 0.9;
			}
			if (z.z > par.doubles.foldingLimit)
			{
				z.z = par.doubles.foldingValue - z.z;
				tgladColor *= 0.9;
			}
			else if (z.z < -par.doubles.foldingLimit)
			{
				z.z = -par.doubles.foldingValue - z.z;
				tgladColor *= 0.9;
			}
			r = z.Length();
		}

		if (par.sphericalFoldingMode)
		{
			double fR2_2 = par.doubles.foldingSphericalFixed * par.doubles.foldingSphericalFixed;
			double mR2_2 = par.doubles.foldingSphericalMin * par.doubles.foldingSphericalMin;
			double r2_2 = r * r;
			double tglad_factor1_2 = fR2_2 / mR2_2;

			if (r2_2 < mR2_2)
			{
				z = z * tglad_factor1_2;
				tgladDE *= tglad_factor1_2;
				tgladColor += 1;
			}
			else if (r2_2 < fR2_2)
			{
				double tglad_factor2_2 = fR2_2 / r2_2;
				z = z * tglad_factor2_2;
				tgladDE *= tglad_factor2_2;
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
				if (Mode == 0)
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
			case trig_optim:
			{
				//optimisation based on: http://www.fractalforums.com/mandelbulb-implementation/realtime-renderingoptimisations/
				double th0 = asin(z.z / r);
				double ph0 = atan2(z.y, z.x);
				double rp = pow(r, p - 1.0);
				double th = th0 * p;
				double ph = ph0 * p;
				double cth = cos(th);
				r_dz = rp * r_dz * p + 1.0;
				rp *= r;
				z = CVector3(cth * cos(ph), cth * sin(ph), sin(th)) * rp + constant;
				r = z.Length();
				break;
			}
			case mandelbulb2:
			{
				double temp, tempR;
				tempR = sqrt(z.x * z.x + z.y * z.y);
				z *= (1.0 / tempR);
				temp = z.x * z.x - z.y * z.y;
				z.y = 2.0 * z.x * z.y;
				z.x = temp;
				z *= tempR;

				tempR = sqrt(z.y * z.y + z.z * z.z);
				z *= (1.0 / tempR);
				temp = z.y * z.y - z.z * z.z;
				z.z = 2.0 * z.y * z.z;
				z.y = temp;
				z *= tempR;

				tempR = sqrt(z.x * z.x + z.z * z.z);
				z *= (1.0 / tempR);
				temp = z.x * z.x - z.z * z.z;
				z.z = 2.0 * z.x * z.z;
				z.x = temp;
				z *= tempR;

				z = z * r;
				z += constant;
				r = z.Length();
				break;
			}
			case mandelbulb3:
			{
				double temp, tempR;

				double sign = 1.0;
				double sign2 = 1.0;

				if (z.x < 0) sign2 = -1.0;
				tempR = sqrt(z.x * z.x + z.y * z.y);
				z *= (1.0 / tempR);
				temp = z.x * z.x - z.y * z.y;
				z.y = 2.0 * z.x * z.y;
				z.x = temp;
				z *= tempR;

				if (z.x < 0) sign = -1.0;
				tempR = sqrt(z.x * z.x + z.z * z.z);
				z *= (1.0 / tempR);
				temp = z.x * z.x - z.z * z.z;
				z.z = 2.0 * z.x * z.z * sign2;
				z.x = temp * sign;
				z *= tempR;

				z = z * r;
				z += constant;
				r = z.Length();
				break;
			}
			case mandelbulb4:
			{
				double rp = pow(r, p - 1);

				double angZ = atan2(z.y, z.x);
				double angY = atan2(z.z, z.x);
				double angX = atan2(z.z, z.y);

				CRotationMatrix rotM;
				rotM.RotateX(angX * (p - 1));
				rotM.RotateY(angY * (p - 1));
				rotM.RotateZ(angZ * (p - 1));

				z = rotM.RotateVector(z) * rp + constant;
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
				double temp;
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

				if (Mode == colouring)
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
				if (par.mandelbox.rotationsEnabled)
				{
					bool lockout = false;
					z = par.mandelbox.rot[0][0].RotateVector(z);
					if (z.x > par.mandelbox.doubles.foldingLimit)
					{
						z.x = par.mandelbox.doubles.foldingValue - z.x;
						tgladColor += par.mandelbox.doubles.colorFactorX;
						lockout = true;
					}
					z = par.mandelbox.rotinv[0][0].RotateVector(z);

					z = par.mandelbox.rot[1][0].RotateVector(z);
					if (!lockout && z.x < -par.mandelbox.doubles.foldingLimit)
					{
						z.x = -par.mandelbox.doubles.foldingValue - z.x;
						tgladColor += par.mandelbox.doubles.colorFactorX;
					}
					z = par.mandelbox.rotinv[1][0].RotateVector(z);

					lockout = false;
					z = par.mandelbox.rot[0][1].RotateVector(z);
					if (z.y > par.mandelbox.doubles.foldingLimit)
					{
						z.y = par.mandelbox.doubles.foldingValue - z.y;
						tgladColor += par.mandelbox.doubles.colorFactorY;
						lockout = true;
					}
					z = par.mandelbox.rotinv[0][1].RotateVector(z);

					z = par.mandelbox.rot[1][1].RotateVector(z);
					if (!lockout && z.y < -par.mandelbox.doubles.foldingLimit)
					{
						z.y = -par.mandelbox.doubles.foldingValue - z.y;
						tgladColor += par.mandelbox.doubles.colorFactorY;
					}
					z = par.mandelbox.rotinv[1][1].RotateVector(z);

					lockout = false;
					z = par.mandelbox.rot[0][2].RotateVector(z);
					if (z.z > par.mandelbox.doubles.foldingLimit)
					{
						z.z = par.mandelbox.doubles.foldingValue - z.z;
						tgladColor += par.mandelbox.doubles.colorFactorZ;
						lockout = true;
					}
					z = par.mandelbox.rotinv[0][2].RotateVector(z);

					z = par.mandelbox.rot[1][2].RotateVector(z);
					if (!lockout && z.z < -par.mandelbox.doubles.foldingLimit)
					{
						z.z = -par.mandelbox.doubles.foldingValue - z.z;
						tgladColor += par.mandelbox.doubles.colorFactorZ;
					}
					z = par.mandelbox.rotinv[1][2].RotateVector(z);
				}
				else
				{
					if (z.x > par.mandelbox.doubles.foldingLimit)
					{
						z.x = par.mandelbox.doubles.foldingValue - z.x;
						tgladColor += par.mandelbox.doubles.colorFactorX;
					}
					else if (z.x < -par.mandelbox.doubles.foldingLimit)
					{
						z.x = -par.mandelbox.doubles.foldingValue - z.x;
						tgladColor += par.mandelbox.doubles.colorFactorX;
					}
					if (z.y > par.mandelbox.doubles.foldingLimit)
					{
						z.y = par.mandelbox.doubles.foldingValue - z.y;
						tgladColor += par.mandelbox.doubles.colorFactorY;
					}
					else if (z.y < -par.mandelbox.doubles.foldingLimit)
					{
						z.y = -par.mandelbox.doubles.foldingValue - z.y;
						tgladColor += par.mandelbox.doubles.colorFactorY;
					}
					if (z.z > par.mandelbox.doubles.foldingLimit)
					{
						z.z = par.mandelbox.doubles.foldingValue - z.z;
						tgladColor += par.mandelbox.doubles.colorFactorZ;
					}
					else if (z.z < -par.mandelbox.doubles.foldingLimit)
					{
						z.z = -par.mandelbox.doubles.foldingValue - z.z;
						tgladColor += par.mandelbox.doubles.colorFactorZ;
					}
				}

				r = z.Length();
				double r2 = r * r;

				if (r2 < mR2)
				{
					z *= tglad_factor1;
					tgladDE *= tglad_factor1;
					tgladColor += par.mandelbox.doubles.colorFactorSp1;
				}
				else if (r2 < fR2)
				{
					double tglad_factor2 = fR2 / r2;
					z *= tglad_factor2;
					tgladDE *= tglad_factor2;
					tgladColor += par.mandelbox.doubles.colorFactorSp2;
				}

				z = par.mandelbox.mainRot.RotateVector(z);

				z = z * scale + constant;
				tgladDE = tgladDE*fabs(scale)+1.0;

				r = z.Length();
				break;
			}
			case smoothMandelbox:
			{
				double sm = par.mandelbox.doubles.sharpness;

				double zk1 = SmoothConditionAGreaterB(z.x, par.mandelbox.doubles.foldingLimit,sm);
				double zk2 = SmoothConditionALessB(z.x, -par.mandelbox.doubles.foldingLimit,sm);
				z.x = z.x * (1.0 - zk1) + (par.mandelbox.doubles.foldingValue - z.x) * zk1;
				z.x = z.x * (1.0 - zk2) + (-par.mandelbox.doubles.foldingValue - z.x) * zk2;
				tgladColor += (zk1 + zk2) * par.mandelbox.doubles.colorFactorX;

				double zk3 = SmoothConditionAGreaterB(z.y, par.mandelbox.doubles.foldingLimit,sm);
				double zk4 = SmoothConditionALessB(z.y, -par.mandelbox.doubles.foldingLimit,sm);
				z.y = z.y * (1.0 - zk3) + (par.mandelbox.doubles.foldingValue - z.y) * zk3;
				z.y = z.y * (1.0 - zk4) + (-par.mandelbox.doubles.foldingValue - z.y) * zk4;
				tgladColor += (zk3 + zk4) * par.mandelbox.doubles.colorFactorY;

				double zk5 = SmoothConditionAGreaterB(z.z, par.mandelbox.doubles.foldingLimit,sm);
				double zk6 = SmoothConditionALessB(z.z, -par.mandelbox.doubles.foldingLimit,sm);
				z.z = z.z * (1.0 - zk5) + (par.mandelbox.doubles.foldingValue - z.z) * zk5;
				z.z = z.z * (1.0 - zk6) + (-par.mandelbox.doubles.foldingValue - z.z) * zk6;
				tgladColor += (zk5 + zk6) * par.mandelbox.doubles.colorFactorZ;

				r = z.Length();
				double r2 = r * r;
				double tglad_factor2 = fR2 / r2;
				double rk1 = SmoothConditionALessB(r2, mR2, sm);
				double rk2 = SmoothConditionALessB(r2, fR2, sm);
				double rk21 = (1.0 - rk1) * rk2;

				z = z * (1.0 - rk1) + z * (tglad_factor1 * rk1);
				z = z * (1.0 - rk21) + z * (tglad_factor2 * rk21);
				tgladDE = tgladDE * (1.0 - rk1) + tgladDE * (tglad_factor1 * rk1);
				tgladDE = tgladDE * (1.0 - rk21) + tgladDE * (tglad_factor2 * rk21);
				tgladColor += rk1 * par.mandelbox.doubles.colorFactorSp1;
				tgladColor += rk21 * par.mandelbox.doubles.colorFactorSp2;

				z = par.mandelbox.mainRot.RotateVector(z);
				z = z * scale + constant;

				tgladDE = tgladDE * fabs(scale) + 1.0;
				r = z.Length();
				break;
			}
			case foldingIntPow2:
			{
				if (z.x > par.doubles.FoldingIntPowFoldFactor) z.x = par.doubles.FoldingIntPowFoldFactor * 2.0 - z.x;
				else if (z.x < -par.doubles.FoldingIntPowFoldFactor) z.x = -par.doubles.FoldingIntPowFoldFactor * 2.0 - z.x;

				if (z.y > par.doubles.FoldingIntPowFoldFactor) z.y = par.doubles.FoldingIntPowFoldFactor * 2.0 - z.y;
				else if (z.y < -par.doubles.FoldingIntPowFoldFactor) z.y = -par.doubles.FoldingIntPowFoldFactor * 2.0 - z.y;

				if (z.z > par.doubles.FoldingIntPowFoldFactor) z.z = par.doubles.FoldingIntPowFoldFactor * 2.0 - z.z;
				else if (z.z < -par.doubles.FoldingIntPowFoldFactor) z.z = -par.doubles.FoldingIntPowFoldFactor * 2.0 - z.z;

				r = z.Length();

				double fR2_2 = 1.0;
				double mR2_2 = 0.25;
				double r2_2 = r * r;
				double tglad_factor1_2 = fR2_2 / mR2_2;

				if (r2_2 < mR2_2)
				{
					z = z * tglad_factor1_2;
				}
				else if (r2_2 < fR2_2)
				{
					double tglad_factor2_2 = fR2_2 / r2_2;
					z = z * tglad_factor2_2;
				}

				z = z * 2.0;
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
				z.z *= par.doubles.FoldingIntPowZfactor;
				r = z.Length();
				break;
			}
			case kaleidoscopic:
			{

				if (par.IFS.absX) z.x = fabs(z.x);
				if (par.IFS.absY) z.y = fabs(z.y);
				if (par.IFS.absZ) z.z = fabs(z.z);

				for (int i = 0; i < IFS_VECTOR_COUNT; i++)
				{
					if (par.IFS.enabled[i])
					{
						z = par.IFS.rot[i].RotateVector(z);
						double length = z.Dot(par.IFS.doubles.direction[i]);

						if (length < par.IFS.doubles.distance[i])
						{
							z -= par.IFS.doubles.direction[i] * (2.0 * (length - par.IFS.doubles.distance[i]) * par.IFS.doubles.intensity[i]);
						}

					}
				}
				z = par.IFS.mainRot.RotateVector(z - par.IFS.doubles.offset) + par.IFS.doubles.offset;

				if (Mode == colouring)
				{
					double length2 = z.Length();
					if (length2 < min) min = length2;
				}

				z *= par.IFS.doubles.scale;
				z -= par.IFS.doubles.offset * (par.IFS.doubles.scale - 1.0);

				tgladDE *= par.IFS.doubles.scale;
				r = z.Length();

				break;
			}
			case mandelboxVaryScale4D:
			{
				/*Scale = Scale + Scale_vary*(abs(Scale)-1)
				x = abs(x+Fold) - abs(x-Fold) - x
				y = abs(y+Fold) - abs(y-Fold) - y
				z = abs(z+Fold) - abs(z-Fold) - z
				w = abs(w+Fold) - abs(w-Fold) - w
				rr = pow(x*x + y*y + z*z + w*w, R_power)
				if rr < sqr(Min_R) then m = Scale/sqr(Min_R) else
				if rr < 1 then m = Scale/rr else m = Scale
				x = x * m + Cx
				y = y * m + Cy
				z = z * m + Cz
				w = w * m + W_add + Cw
				*/

				scale = scale + par.mandelbox.doubles.vary4D.scaleVary * (fabs(scale) - 1.0);
				z.x = fabs(z.x + par.mandelbox.doubles.vary4D.fold) - fabs(z.x - par.mandelbox.doubles.vary4D.fold) - z.x;
				z.y = fabs(z.y + par.mandelbox.doubles.vary4D.fold) - fabs(z.y - par.mandelbox.doubles.vary4D.fold) - z.y;
				z.z = fabs(z.z + par.mandelbox.doubles.vary4D.fold) - fabs(z.z - par.mandelbox.doubles.vary4D.fold) - z.z;
				w = fabs(w + par.mandelbox.doubles.vary4D.fold) - fabs(w - par.mandelbox.doubles.vary4D.fold) - w;
				double rr = pow(z.x * z.x + z.y * z.y + z.z * z.z + w * w, par.mandelbox.doubles.vary4D.rPower);
				double m = scale;
				if (rr < par.mandelbox.doubles.vary4D.minR*par.mandelbox.doubles.vary4D.minR) m = scale / (par.mandelbox.doubles.vary4D.minR * par.mandelbox.doubles.vary4D.minR);
				else if (rr < 1.0) m = scale / rr;
				z = z * m + constant;
				w = w * m + par.mandelbox.doubles.vary4D.wadd;
				tgladDE = tgladDE * fabs(m) + 1.0;
				r = sqrt(z.x * z.x + z.y * z.y + z.z * z.z + w * w);
				break;
			}
			case hybrid:
				break;
			case none:
				break;
		}

		//************************** iteration terminate conditions *****************
		if (Mode == deltaDE1)
		{
			if (r > 1e10)
				break;
		}
		else if (Mode == deltaDE2)
		{
			if (L == *iter_count)
				break;
		}

		if (actualFormula == menger_sponge || actualFormula == kaleidoscopic)
		{
			if (r > 1000)
			{
				distance = (r - 2.0) / tgladDE;
				break;
			}
		}
		else if (actualFormula == tglad || actualFormula == smoothMandelbox || actualFormula == mandelboxVaryScale4D)
		{
			if (r > 1024)
			{
				distance = r / fabs(tgladDE);
				break;
			}
		}
		else
		{
			if (Mode == normal) //condition for all other trigonometric and hypercomplex fractals
			{
				if (r > 1e2)
				{
					distance = 0.5 * r * log(r) / r_dz;
					break;
				}
			}
			else if (Mode == fake_AO) //mode 2
			{
				if (r < min) min = r;
				if (r > 1e15)
				{
					distance = min;
					break;
				}
			}
			else if (Mode == colouring) //mode 1
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
		histogram[L]++;
	else
		histogram[63]++;

	if (iter_count != NULL)
		*iter_count = L;

	if (Mode == normal)
	{
		if (L == par.N)
			distance = 0;
		return distance;
	}

	if (Mode == deltaDE1 || Mode == deltaDE2)
		return r;

	if (Mode == fake_AO)
		return distance;

	if (Mode == colouring)
	{
		if (par.formula == hybrid)
		{
			if (min > 100) 
				min = 100;
			if (distance > 20) 
				distance = 20;
			if (tgladColor > 1000) 
				tgladColor = 1000;

			return distance * 5000.0 + tgladColor * 100.0 + min * 1000.0;
		} 
		else if (actualFormula == tglad || actualFormula == smoothMandelbox)
			return tgladColor * 100.0 + z.Length()*par.mandelbox.doubles.colorFactorR;
		else if (actualFormula == kaleidoscopic || actualFormula == menger_sponge)
			return min * 1000.0;
		else
			return distance * 5000.0;
	}
}

//******************* Calculate distance *******************8

double CalculateDistance(CVector3 point, const sFractal &params, bool *max_iter)
{
	int L;
	double distance;
	if (params.limits_enabled)
	{
		bool limit = false;
		double distance_a = 0;
		double distance_b = 0;
		double distance_c = 0;

		if (point.x < params.doubles.amin)
		{
			distance_a = fabs(params.doubles.amin - point.x) + params.doubles.detailSize;
			limit = true;
		}
		if (point.x > params.doubles.amax)
		{
			distance_a = fabs(params.doubles.amax - point.x) + params.doubles.detailSize;
			limit = true;
		}

		if (point.y < params.doubles.bmin)
		{
			distance_a = fabs(params.doubles.bmin - point.y) + params.doubles.detailSize;
			limit = true;
		}
		if (point.y > params.doubles.bmax)
		{
			distance_b = fabs(params.doubles.bmax - point.y) + params.doubles.detailSize;
			limit = true;
		}

		if (point.z < params.doubles.cmin)
		{
			distance_c = fabs(params.doubles.cmin - point.z) + params.doubles.detailSize;
			limit = true;
		}
		if (point.z > params.doubles.cmax)
		{
			distance_c = fabs(params.doubles.cmax - point.z) + params.doubles.detailSize;
			limit = true;
		}

		if (limit)
		{
			if (max_iter != NULL)
				*max_iter = false;
			distance = dMax(distance_a, distance_b, distance_c);
			return distance;
		}
	}

	if (params.analitycDE)
	{
		distance = Compute<normal>(point, params, &L);
		if (max_iter != NULL)
		{
			if (L == params.N)
				*max_iter = true;
			else
				*max_iter = false;
		}

		if (L < params.minN && distance < params.doubles.detailSize)
			distance = params.doubles.detailSize;
		
		if (params.interiorMode)
		{
			if (distance < 0.5 * params.doubles.detailSize || L == params.N)
			{
				distance = params.doubles.detailSize;
				if (max_iter != NULL)
					*max_iter = false;
			}
		}

		if (distance < 0)
			distance = 0;

		return distance;
	}
	else
	{
		double deltaDE = 1e-10;

		double r = Compute<deltaDE1>(point, params, &L);
		int retval = L;

		point.x += deltaDE;
		point.y += 0;
		point.z += 0;
		double r2 = Compute<deltaDE2>(point, params, &L);
		double dr1 = fabs(r2 - r) / deltaDE;

		point.x -= deltaDE;
		point.y += deltaDE;
		point.z += 0;
		r2 = Compute<deltaDE2>(point, params, &L);
		double dr2 = fabs(r2 - r) / deltaDE;

		point.x += 0;
		point.y -= deltaDE;
		point.z += deltaDE;
		r2 = Compute<deltaDE2>(point, params, &L);
		double dr3 = fabs(r2 - r) / deltaDE;

		double dr = sqrt(dr1 * dr1 + dr2 * dr2 + dr3 * dr3);

		if(params.linearDEmode)
		{
			distance = 0.5 * r / dr;
		}
		else
		{
			distance = 0.5 * r * log(r) / dr;
		}

		if (retval == params.N)
		{
			if (max_iter != NULL)
				*max_iter = true;
			distance = 0;
		}
		else if (max_iter != NULL)
			*max_iter = false;

		if (L < params.minN && distance < params.doubles.detailSize)
			distance = params.doubles.detailSize;

		if (params.interiorMode)
		{
			if (distance < 0.5 * params.doubles.detailSize || retval == 256)
			{
				distance = params.doubles.detailSize;
				if (max_iter != NULL)
					*max_iter = false;
			}
		}

		if (distance < 0) 
			distance = 0;

		return distance;
	}
}

// force template instantiation
template double Compute<normal>(CVector3, const sFractal&, int*);
template double Compute<colouring>(CVector3, const sFractal&, int*);
template double Compute<fake_AO>(CVector3, const sFractal&, int*);
template double Compute<deltaDE1>(CVector3, const sFractal&, int*);
template double Compute<deltaDE2>(CVector3, const sFractal&, int*);
