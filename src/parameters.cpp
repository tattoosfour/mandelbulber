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
#include "parameters.hpp"
#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>

#define _PARAM_DEBUG

using namespace parameters;

container::container()
{
	myMap.clear();
}

container::~container()
{
	myMap.clear();
}

template <class T>
void container::addParam(std::string name, T defaultVal)
{
	record newRecord;
	sMultiVal multi;
	newRecord.type = Assigner(multi, defaultVal);
	newRecord.actualVal = multi;
	newRecord.defaultVal = multi;

	std::pair<std::map<std::string,record>::iterator,bool> ret;
	ret = myMap.insert(std::pair<std::string,record>(name,newRecord));

  if (ret.second==false) {
    std::cerr << "addParam(): element '" << name << "' already existed" << std::endl;
  }
}

// force template instantiation
template void container::addParam<double>(std::string name, double defaultVal);
template void container::addParam<int>(std::string name, int defaultVal);
template void container::addParam<std::string>(std::string name, std::string defaultVal);
template void container::addParam<CVector3>(std::string name, CVector3 defaultVal);
template void container::addParam<sRGB>(std::string name, sRGB defaultVal);
template void container::addParam<bool>(std::string name, bool defaultVal);

varType container::Assigner(sMultiVal &multi, double val)
{
	multi.dVal = val;
	multi.iVal = val;
	multi.vVal = CVector3(val, val, val);
	char sbuff[100];
	snprintf(sbuff, 100, "%.16lg", val);
	multi.sVal = std::string(sbuff);
	multi.cVal = (sRGB) {0, 0, 0};
	multi.bVal = val;
	return typeDouble;
}

varType container::Assigner(sMultiVal &multi, int val)
{
	multi.dVal = val;
	multi.iVal = val;
	multi.vVal = CVector3(val, val, val);
	char sbuff[100];
	snprintf(sbuff, 100, "%d", val);
	multi.sVal = std::string(sbuff);
	multi.cVal.R = (val / 65536) * 256;
	multi.cVal.G = ((val / 256) % 256) * 256;
	multi.cVal.B = (val % 256) * 256;
	multi.bVal = val;
	return typeInt;
}

varType container::Assigner(sMultiVal &multi, std::string val)
{
	multi.dVal = atof(val.c_str());
	multi.iVal = atoi(val.c_str());
	sscanf(val.c_str(), "%lf %lf %lf", &multi.vVal.x, &multi.vVal.y, &multi.vVal.z);
	multi.sVal = val;
	sscanf(val.c_str(), "%x %x %x", &multi.cVal.R, &multi.cVal.G, &multi.cVal.B);
	multi.bVal = multi.iVal;
	return typeString;
}

varType container::Assigner(sMultiVal &multi, CVector3 val)
{
	multi.dVal = val.Length();
	multi.iVal = val.Length();
	multi.vVal = val;
	char sbuff[100];
	snprintf(sbuff, 100, "%.16lg %.16lg %.16lg", val.x, val.y, val.z);
	multi.sVal = std::string(sbuff);
	multi.cVal.R = val.x;
	multi.cVal.G = val.y;
	multi.cVal.B = val.z;
	multi.bVal = false;
	return typeVector3;
}

varType container::Assigner(sMultiVal &multi, sRGB val)
{
	multi.dVal = 0.0;
	multi.iVal = val.B + val.G * 256 + val.R * 65536;
	multi.vVal = CVector3(val.R, val.G, val.B);
	char sbuff[100];
	snprintf(sbuff, 100, "%x %x %x", val.R, val.G, val.B);
	multi.sVal = std::string(sbuff);
	multi.cVal = val;
	multi.bVal = false;
	return typeRgb;
}

varType container::Assigner(sMultiVal &multi, bool val)
{
	multi.dVal = val;
	multi.iVal = val;
	multi.vVal = CVector3(val, val, val);
	char sbuff[100];
	snprintf(sbuff, 100, "%d", val);
	multi.sVal = std::string(sbuff);
	multi.cVal = (sRGB) {0, 0, 0};
	multi.bVal = val;
	return typeBool;
}

varType container::Getter(sMultiVal multi, double &val)
{
	val = multi.dVal;
	return typeDouble;
}

varType container::Getter(sMultiVal multi, int &val)
{
	val = multi.iVal;
	return typeInt;
}

varType container::Getter(sMultiVal multi, CVector3 &val)
{
	val = multi.vVal;
	return typeVector3;
}

varType container::Getter(sMultiVal multi, std::string &val)
{
	val = multi.sVal;
	return typeString;
}

varType container::Getter(sMultiVal multi, sRGB &val)
{
	val = multi.cVal;
	return typeRgb;
}

varType container::Getter(sMultiVal multi, bool &val)
{
	val = multi.bVal;
	return typeBool;
}

template <class T>
void container::Set(std::string name, T val)
{
	std::map<std::string, record>::iterator it;
	it = myMap.find(name);
	if(it != myMap.end())
	{
		sMultiVal multi;
		varType type = Assigner(multi, val);
		if(it->second.type == type)
		{
			it->second.actualVal = multi;
		}
#ifdef _PARAM_DEBUG
		else
		{
			std::cerr << "Set(): element '" << name << "' got value of wrong type" << std::endl;
		}
#endif
	}
	else
	{
		std::cerr << "Set(): element '" << name << "' doesn't exists" << std::endl;
	}
}
template void container::Set<double>(std::string name, double val);
template void container::Set<int>(std::string name, int val);
template void container::Set<std::string>(std::string name, std::string val);
template void container::Set<CVector3>(std::string name, CVector3 val);
template void container::Set<sRGB>(std::string name, sRGB val);
template void container::Set<bool>(std::string name, bool val);

template <class T>
T container::Get(std::string name)
{
	std::map<std::string, record>::iterator it;
	it = myMap.find(name);
	T val;
	if(it != myMap.end())
	{
		record rec = it->second;
		sMultiVal multi = rec.actualVal;
		varType type = Getter(multi, val);
#ifdef _PARAM_DEBUG
		if(it->second.type != type)
		{
			std::cerr << "Get(): element '" << name << "' gave value of not default type" << std::endl;
		}
#endif
	}
	else
	{
		std::cerr << "Get(): element '" << name << "' doesn't exists" << std::endl;
	}
	return val;
}
template double container::Get<double>(std::string name);
template int container::Get<int>(std::string name);
template std::string container::Get<std::string>(std::string name);
template CVector3 container::Get<CVector3>(std::string name);
template sRGB container::Get<sRGB>(std::string name);
template bool container::Get<bool>(std::string name);

void container::DebugPrintf(std::string name)
{
	record rec = myMap[name];
	printf("Actual value of variable '%s'\n", name.c_str());
	printf("int = %d\n", rec.actualVal.iVal);
	printf("double = %.16lg\n", rec.actualVal.dVal);
	printf("vector3 = %.16lg; %.16lg; %.16lg\n", rec.actualVal.vVal.x, rec.actualVal.vVal.y, rec.actualVal.vVal.z);
	printf("string = %s\n", rec.actualVal.sVal.c_str());
	printf("color = %x %x %x\n", rec.actualVal.cVal.R, rec.actualVal.cVal.G, rec.actualVal.cVal.B);
	switch(rec.type)
	{
		case typeInt:
			printf("variable type 'int'\n");
			break;
		case typeDouble:
			printf("variable type 'double'\n");
			break;
		case typeVector3:
			printf("variable type 'vector3'\n");
			break;
		case typeString:
			printf("variable type 'string'\n");
			break;
		case typeRgb:
			printf("variable type 'rgb'\n");
			break;
		case typeBool:
			printf("variable type 'bool'\n");
			break;
	}
}
