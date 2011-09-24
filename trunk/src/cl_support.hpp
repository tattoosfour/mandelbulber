/*
 * cl_support.hpp
 *
 *  Created on: 18-09-2011
 *      Author: krzysztof
 */

#ifndef CL_SUPPORT_HPP_
#define CL_SUPPORT_HPP_

#define CL_WIDTH 800
#define CL_HEIGHT 600

#define __NO_STD_VECTOR // Use cl::vector instead of STL version
#include <CL/cl.hpp>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <iterator>

typedef cl_float4 float4;
#include "cl_data.h"

class CclSupport
{
public:
	CclSupport();
	void Init(void);
	void SetParams(sClParams ClParams, sClFractal ClFractal);
	void Render(void);
	unsigned char* GetRgbBuff() {return rgbbuff;}
	bool IsReady(void) {return ready;}
	bool IsEnabled(void) {return enabled;}
	void Enable(void);
	void Disable(void) {enabled = false;}
private:
	bool enabled;
	bool ready;
	unsigned char *rgbbuff;

	cl::vector<cl::Platform> platformList;
	std::string platformVendor;
	cl::Context *context;
	cl::Buffer *outCL;
	cl::vector<cl::Device> devices;
	cl::Program::Sources *source;
	cl::Program *program;
	cl::Kernel *kernel;
	cl::CommandQueue *queue;

	unsigned int buffSize;
	cl_int numberOfComputeUnits;
	cl_int maxWorkItemDimmensions;
	cl_int maxMaxWorkGroupSize[3];
	cl_int maxClockFrequency;
	size_t memorySize;
	size_t workGroupSize;
};

extern CclSupport *clSupport;

#endif /* CL_SUPPORT_HPP_ */
