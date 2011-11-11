/*
 * cl_support.hpp
 *
 *  Created on: 18-09-2011
 *      Author: krzysztof
 */

#ifndef CL_SUPPORT_HPP_
#define CL_SUPPORT_HPP_

#define __NO_STD_VECTOR // Use cl::vector instead of STL version
#include <CL/cl.hpp>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <iterator>

#include "cimage.hpp"

#include "cl_data.h"

class CclSupport
{
public:
	CclSupport();
	void Init(void);
	void SetParams(sClParams ClParams, sClFractal ClFractal);
	void Render(cImage *image, GtkWidget *outputDarea);
	sClPixel * GetRgbBuff() {return rgbbuff;}
	bool IsReady(void) {return ready;}
	bool IsEnabled(void) {return enabled;}
	void Enable(void);
	void Disable(void);
	int GetWidth() {return width;}
	int GetHeight() {return height;}
	void SetSize(int w, int h){width = w; height = h;}
private:
	bool enabled;
	bool ready;
	sClPixel *rgbbuff;

	cl::vector<cl::Platform> platformList;
	std::string platformVendor;
	cl::Context *context;
	cl::Buffer *outCL;
	cl::vector<cl::Device> devices;
	cl::Program::Sources *source;
	cl::Program::Sources *source2;
	cl::Program *program;
	cl::Kernel *kernel;
	cl::CommandQueue *queue;

	int width;
	int height;
	unsigned int buffSize;
	unsigned int stepSize;
	unsigned int steps;
	cl_int numberOfComputeUnits;
	cl_int maxWorkItemDimmensions;
	cl_int maxMaxWorkGroupSize[3];
	cl_int maxClockFrequency;
	size_t memorySize;
	size_t workGroupSize;
};

extern CclSupport *clSupport;

#endif /* CL_SUPPORT_HPP_ */
