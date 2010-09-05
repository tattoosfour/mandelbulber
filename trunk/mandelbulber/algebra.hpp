/********************************************************
 /                   3D math                             *
 /                                                       *
 / author: Krzysztof Marczak                             *
 / contact: buddhi1980@gmail.com                         *
 / licence: GNU GPL                                      *
 ********************************************************/

#ifndef ALGEBRA_H_
#define ALGEBRA_H_

#include <math.h>

/************************* vector 3D **********************/
class CVector3
{
public:
	CVector3();
	CVector3(double x_init, double y_init, double z_init);
	CVector3(double alfa, double beta);
	CVector3(const CVector3 &vector);
	CVector3 operator+(const CVector3&);
	CVector3 operator-(const CVector3&);
	CVector3 operator*(const double &scalar);
	CVector3& operator=(const CVector3&);
	CVector3& operator+=(const CVector3&);
	CVector3& operator-=(const CVector3&);
	CVector3& operator*=(const double&);
	double Length(void);
	double Dot(CVector3);
	double Normalize(void); //returns normalization factor
	double GetAlfa(void);
	double GetBeta(void);
	double x;
	double y;
	double z;
};

/************************* matrix 3x3 (fast) *****************/;
class CMatrix33
{
public:
	CMatrix33();
	CMatrix33(const CMatrix33 &matrix);
	CMatrix33 operator*(CMatrix33 &matrix);
	CVector3 operator*(CVector3 &vector);
	CMatrix33& operator=(const CMatrix33&);
	double m11;
	double m12;
	double m13;
	double m21;
	double m22;
	double m23;
	double m31;
	double m32;
	double m33;
};

/************************* rotation matrix *******************/
class CRotationMatrix
{
public:
	CRotationMatrix();
	void RotateX(double angle);
	void RotateY(double angle);
	void RotateZ(double angle);
	void Null();
	CVector3 RotateVector(CVector3 vector);
	double GetAlfa();
	double GetBeta();
	double GetGamma();

private:
	CMatrix33 matrix;
	bool zero;
};

#endif /* ALGEBRA_H_ */
