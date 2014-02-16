/*********************************************************
 /                   MANDELBULBER
 / class for parameter handling
 /
 /
 / author: Krzysztof Marczak
 / contact: buddhi1980@gmail.com
 / licence: GNU GPL v3.0
 /
 ********************************************************/

#ifndef SHADERS_HPP_
#define SHADERS_HPP_

#include <string>
#include <map>
#include "algebra.hpp"
#include "cimage.hpp"

namespace parameters
{
enum varType
{
	typeInt, typeDouble, typeString, typeVector3, typeRgb, typeBool
};

struct sMultiVal
{
	double dVal;
	int iVal;
	CVector3 vVal;
	std::string sVal;
	sRGB cVal;
	bool bVal;
};

struct record
{
	varType type;
	sMultiVal actualVal;
	sMultiVal defaultVal;
	sMultiVal minVal;
	sMultiVal maxVal;
	bool morphable;
	bool limitsDefined;
};

class container
{
public:
	container();
	~container();
	template <class T> void addParam(std::string name, T defaultVal, bool morphable);
	template <class T> void addParam(std::string name, T defaultVal, T minVal, T maxVal, bool morphable);
	template <class T> void addParam(std::string name, int index, T defaultVal, bool morphable);
	template <class T> void addParam(std::string name, int index, T defaultVal, T minVal, T maxVal, bool morphable);
	template <class T> void Set(std::string name, T val);
	template <class T> T Get(std::string name);

	void DebugPrintf(std::string name);

private:
	varType Assigner(sMultiVal &multi, double val);
	varType Assigner(sMultiVal &multi, int val);
	varType Assigner(sMultiVal &multi, std::string val);
	varType Assigner(sMultiVal &multi, CVector3 val);
	varType Assigner(sMultiVal &multi, sRGB val);
	varType Assigner(sMultiVal &multi, bool val);
	varType Getter(sMultiVal multi, double &val);
	varType Getter(sMultiVal multi, int &val);
	varType Getter(sMultiVal multi, std::string &val);
	varType Getter(sMultiVal multi, CVector3 &val);
	varType Getter(sMultiVal multi, sRGB &val);
	varType Getter(sMultiVal multi, bool &val);
	std::string nameWithIndex(std::string *str, int index);

	std::map<std::string, record> myMap;
};

}
#endif /*SHADERS_HPP_*/
