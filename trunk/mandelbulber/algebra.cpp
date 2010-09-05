/*
 * algebra.cpp
 *
 *  Created on: 2010-02-28
 *      Author: krzysztof
 */
#include "algebra.hpp"

//************************ class CVector3 ****************************

//constructors
CVector3::CVector3()
{
	x = 0;
	y = 0;
	z = 0;
}

CVector3::CVector3(double x_init, double y_init, double z_init)
{
	x = x_init;
	y = y_init;
	z = z_init;
}

CVector3::CVector3(double alfa, double beta)
{
	x = cos(beta) * cos(alfa);
	y = cos(beta) * sin(alfa);
	z = sin(beta);
}

CVector3::CVector3(const CVector3 &vector)
{
	x = vector.x;
	y = vector.y;
	z = vector.z;
}

CVector3& CVector3::operator=(const CVector3 &vector)
{
	x = vector.x;
	y = vector.y;
	z = vector.z;
	return *this;
}

//operators

CVector3 CVector3::operator+(const CVector3 &vector)
{
	CVector3 result(x + vector.x, y + vector.y, z + vector.z);
	return result;
}

CVector3 CVector3::operator-(const CVector3 &vector)
{
	CVector3 result(x - vector.x, y - vector.y, z - vector.z);
	return result;
}

CVector3 CVector3::operator*(const double &scalar)
{
	CVector3 result(x * scalar, y * scalar, z * scalar);
	return result;
}

CVector3& CVector3::operator+=(const CVector3 &vector)
{
	x += vector.x;
	y += vector.y;
	z += vector.z;
	return *this;
}

CVector3& CVector3::operator-=(const CVector3 &vector)
{
	x -= vector.x;
	y -= vector.y;
	z -= vector.z;
	return *this;
}

CVector3& CVector3::operator*=(const double &scalar)
{
	x *= scalar;
	y *= scalar;
	z *= scalar;
	return *this;
}

//methods

double CVector3::Length()
{
	return sqrt(x * x + y * y + z * z);
}

double CVector3::Dot(CVector3 vector)
{
	return x * vector.x + y * vector.y + z * vector.z;
}

double CVector3::Normalize()
{
	double norm = 1.0 / Length();
	x = x * norm;
	y = y * norm;
	z = z * norm;
	return norm;
}

double CVector3::GetAlfa()
{
	return atan2(y, x);
}

double CVector3::GetBeta()
{
	return atan2(z, sqrt(x * x + y * y));
}

/***************** class CMatrix33 ***********************/
CMatrix33::CMatrix33()
{
	m11 = 0;
	m12 = 0;
	m13 = 0;
	m21 = 0;
	m22 = 0;
	m23 = 0;
	m31 = 0;
	m32 = 0;
	m33 = 0;
}

CMatrix33::CMatrix33(const CMatrix33 &matrix)
{
	m11 = matrix.m11;
	m12 = matrix.m12;
	m13 = matrix.m13;
	m21 = matrix.m21;
	m22 = matrix.m22;
	m23 = matrix.m23;
	m31 = matrix.m31;
	m32 = matrix.m32;
	m33 = matrix.m33;
}

CMatrix33& CMatrix33::operator=(const CMatrix33 &matrix)
{
	m11 = matrix.m11;
	m12 = matrix.m12;
	m13 = matrix.m13;
	m21 = matrix.m21;
	m22 = matrix.m22;
	m23 = matrix.m23;
	m31 = matrix.m31;
	m32 = matrix.m32;
	m33 = matrix.m33;
	return *this;
}

CMatrix33 CMatrix33::operator*(CMatrix33 &matrix)
{
	CMatrix33 result;
	result.m11 = m11 * matrix.m11 + m12 * matrix.m21 + m13 * matrix.m31;
	result.m12 = m11 * matrix.m12 + m12 * matrix.m22 + m13 * matrix.m32;
	result.m13 = m11 * matrix.m13 + m12 * matrix.m23 + m13 * matrix.m33;
	result.m21 = m21 * matrix.m11 + m22 * matrix.m21 + m23 * matrix.m31;
	result.m22 = m21 * matrix.m12 + m22 * matrix.m22 + m23 * matrix.m32;
	result.m23 = m21 * matrix.m13 + m22 * matrix.m23 + m23 * matrix.m33;
	result.m31 = m31 * matrix.m11 + m32 * matrix.m21 + m33 * matrix.m31;
	result.m32 = m31 * matrix.m12 + m32 * matrix.m22 + m33 * matrix.m32;
	result.m33 = m31 * matrix.m13 + m32 * matrix.m23 + m33 * matrix.m33;
	return result;
}

CVector3 CMatrix33::operator*(CVector3 &vector)
{
	CVector3 result;
	result.x = m11 * vector.x + m12 * vector.y + m13 * vector.z;
	result.y = m21 * vector.x + m22 * vector.y + m23 * vector.z;
	result.z = m31 * vector.x + m32 * vector.y + m33 * vector.z;
	return result;
}

/**************** class RotarionMatrix **********************/
CRotationMatrix::CRotationMatrix()
{
	matrix.m11 = 1.0;
	matrix.m12 = 0.0;
	matrix.m13 = 0.0;
	matrix.m21 = 0.0;
	matrix.m22 = 1.0;
	matrix.m23 = 0.0;
	matrix.m31 = 0.0;
	matrix.m32 = 0.0;
	matrix.m33 = 1.0;
	zero = true;
}

void CRotationMatrix::RotateX(double angle)
{
	if (angle != 0.0)
	{
		CMatrix33 rot;
		double s = sin(angle);
		double c = cos(angle);
		rot.m11 = 1.0;
		rot.m22 = c;
		rot.m33 = c;
		rot.m23 = -s;
		rot.m32 = s;
		matrix = matrix * rot;
		zero = false;
	}
}

void CRotationMatrix::RotateY(double angle)
{
	if (angle != 0.0)
	{
		CMatrix33 rot;
		double s = sin(angle);
		double c = cos(angle);
		rot.m22 = 1.0;
		rot.m33 = c;
		rot.m11 = c;
		rot.m31 = -s;
		rot.m13 = s;
		matrix = matrix * rot;
		zero = false;
	}
}

void CRotationMatrix::RotateZ(double angle)
{
	if (angle != 0.0)
	{
		CMatrix33 rot;
		double s = sin(angle);
		double c = cos(angle);
		rot.m33 = 1.0;
		rot.m11 = c;
		rot.m22 = c;
		rot.m12 = -s;
		rot.m21 = s;
		matrix = matrix * rot;
		zero = false;
	}
}

CVector3 CRotationMatrix::RotateVector(CVector3 vector)
{
	if (!zero)
	{
		CVector3 vector2 = matrix * vector;
		return vector2;
	}
	else
	{
		return vector;
	}
}

void CRotationMatrix::Null(void)
{
	//CRotationMatrix();
	matrix.m11 = 1.0;
	matrix.m12 = 0.0;
	matrix.m13 = 0.0;
	matrix.m21 = 0.0;
	matrix.m22 = 1.0;
	matrix.m23 = 0.0;
	matrix.m31 = 0.0;
	matrix.m32 = 0.0;
	matrix.m33 = 1.0;
	zero = true;
}

double CRotationMatrix::GetAlfa(void)
{
	return atan2(matrix.m12,matrix.m22);
}

double CRotationMatrix::GetBeta(void)
{
	return asin(-matrix.m32);
}

double CRotationMatrix::GetGamma(void)
{
	return atan2(matrix.m31,matrix.m33);
}


