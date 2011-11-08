/*
 * cl_support.cpp
 *
 *  Created on: 18-09-2011
 *      Author: krzysztof marczak
 *      some small parts of code taken from: http://www.codeproject.com/KB/GPU-Programming/IntroToOpenCL.aspx
 */

#include "cl_support.hpp"
#include <string>
#include "interface.h"

CclSupport *clSupport;

void checkErr(cl_int err, const char * name)
{
	if (err != CL_SUCCESS)
	{
		std::cerr << "ERROR: " << name << " (" << err << ")" << std::endl;
		exit(EXIT_FAILURE);
	}
}


CclSupport::CclSupport(void)
{
	enabled = false;
	ready = false;
}

void CclSupport::Init(void)
{
	cl_int err;
	cl::Platform::get(&platformList);
	checkErr(platformList.size() != 0 ? CL_SUCCESS : -1, "cl::Platform::get");
	std::cout << "OpenCL Platform number is: " << platformList.size() << std::endl;

	platformList[0].getInfo((cl_platform_info) CL_PLATFORM_VENDOR, &platformVendor);
	std::cout << "OpenCL Platform is by: " << platformVendor << "\n";
	cl_context_properties cprops[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties) (platformList[0])(), 0 };

	context = new cl::Context(CL_DEVICE_TYPE_GPU, cprops, NULL, NULL, &err);
	checkErr(err, "Context::Context()");
	printf("OpenCL contexts created\n");

	buffSize = CL_WIDTH * CL_HEIGHT * 3;
	rgbbuff = new unsigned char[buffSize];
	outCL = new cl::Buffer(*context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR, buffSize,rgbbuff,&err);
	checkErr(err, "Buffer::Buffer()");
	printf("OpenCL buffer created\n");

	devices = context->getInfo<CL_CONTEXT_DEVICES>();
	checkErr(devices.size() > 0 ? CL_SUCCESS : -1, "devices.size() > 0");

	devices[0].getInfo(CL_DEVICE_MAX_COMPUTE_UNITS, &numberOfComputeUnits);;
	printf("OpenCL Number of compute units: %d\n", numberOfComputeUnits);

	devices[0].getInfo(CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, &maxWorkItemDimmensions);
	printf("OpenCL Max work item dimmensions: %d\n", maxWorkItemDimmensions);

	devices[0].getInfo(CL_DEVICE_MAX_WORK_GROUP_SIZE, &maxMaxWorkGroupSize);
	printf("OpenCL Max work group size: %d\n", maxMaxWorkGroupSize[0]);

	devices[0].getInfo(CL_DEVICE_MAX_CLOCK_FREQUENCY, &maxClockFrequency);
	printf("OpenCL Max clock frequency  %d MHz\n", maxClockFrequency);

	devices[0].getInfo(CL_DEVICE_GLOBAL_MEM_SIZE, &memorySize);
	printf("OpenCL Memory size  %ld MB\n", memorySize/1024/1024);

	std::string clDir = std::string(sharedDir) + "cl/";

	sParamRender params;
	ReadInterface(&params);
	std::string strFormula = "mandelbulb";

	if(params.fractal.formula == trig_optim) strFormula = "mandelbulb";
	if(params.fractal.formula == trig_DE) strFormula = "mandelbulb2";
	if(params.fractal.formula == tglad) strFormula = "mandelbox";

	std::string strFileEngine = clDir + "cl_engine.cl";
	std::ifstream fileEngine(strFileEngine.c_str());
	checkErr(fileEngine.is_open() ? CL_SUCCESS : -1, ("Can't open file:" + strFileEngine).c_str());

	std::string strFileFormulaBegin;
	if(params.fractal.juliaMode) strFileFormulaBegin  = clDir + "cl_formulaBegin" + "Julia" + ".cl";
	else  strFileFormulaBegin = clDir + "cl_formulaBegin" + ".cl";
	std::ifstream fileFormulaBegin(strFileFormulaBegin.c_str());
	checkErr(fileFormulaBegin.is_open() ? CL_SUCCESS : -1, ("Can't open file:" + strFileFormulaBegin).c_str());

	std::string strFileFormulaInit = clDir + "cl_" + strFormula + "Init.cl";
	std::ifstream fileFormulaInit(strFileFormulaInit.c_str());
	checkErr(fileFormulaInit.is_open() ? CL_SUCCESS : -1, ("Can't open file:" + strFileFormulaInit).c_str());

	std::string strFileFormulaFor = clDir + "cl_formulaFor.cl";
	std::ifstream fileFormulaFor(strFileFormulaFor.c_str());
	checkErr(fileFormulaFor.is_open() ? CL_SUCCESS : -1, ("Can't open file:" + strFileFormulaFor).c_str());

	std::string strFileFormula = clDir + "cl_" + strFormula + ".cl";
	std::ifstream fileFormula(strFileFormula.c_str());
	checkErr(fileFormula.is_open() ? CL_SUCCESS : -1, ("Can't open file:" + strFileFormula).c_str());

	std::string strFileFormulaEnd = clDir + "cl_formulaEnd.cl";
	std::ifstream fileFormulaEnd(strFileFormulaEnd.c_str());
	checkErr(fileFormulaEnd.is_open() ? CL_SUCCESS : -1, ("Can't open file:" + strFileFormulaEnd).c_str());

	std::string progEngine(std::istreambuf_iterator<char>(fileEngine), (std::istreambuf_iterator<char>()));
	std::string progFormulaBegin(std::istreambuf_iterator<char>(fileFormulaBegin), (std::istreambuf_iterator<char>()));
	std::string progFormulaInit(std::istreambuf_iterator<char>(fileFormulaInit), (std::istreambuf_iterator<char>()));
	std::string progFormulaFor(std::istreambuf_iterator<char>(fileFormulaFor), (std::istreambuf_iterator<char>()));
	std::string progFormula(std::istreambuf_iterator<char>(fileFormula), (std::istreambuf_iterator<char>()));
	std::string progFormulaEnd(std::istreambuf_iterator<char>(fileFormulaEnd), (std::istreambuf_iterator<char>()));

	cl::Program::Sources sources;
	sources.push_back(std::make_pair(progEngine.c_str(), progEngine.length()));
	sources.push_back(std::make_pair(progFormulaBegin.c_str(), progFormulaBegin.length()));
	sources.push_back(std::make_pair(progFormulaInit.c_str(), progFormulaInit.length()));
	sources.push_back(std::make_pair(progFormulaFor.c_str(), progFormulaFor.length()));
	sources.push_back(std::make_pair(progFormula.c_str(), progFormula.length()));
	sources.push_back(std::make_pair(progFormulaEnd.c_str(), progFormulaEnd.length()+1));
	printf("OpenCL Number of loaded sources %d\n", sources.size());

	program = new cl::Program(*context, sources, &err);
	checkErr(err, "Program()");
	//program->getInfo(CL_PROGRAM_SOURCE, )
	//std::cout << "Program source:\t" << program->getInfo<CL_PROGRAM_SOURCE>() << std::endl;

	err = program->build(devices, "-w");
	std::cout << "OpenCL Build log:\t" << program->getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[0]) << std::endl;
	checkErr(err, "Program::build()");
	printf("OpenCL program built done\n");

	kernel = new cl::Kernel(*program, "fractal3D", &err);
	checkErr(err, "Kernel::Kernel()");
	printf("OpenCL kernel opened\n");

	kernel->getWorkGroupInfo(devices[0], CL_KERNEL_WORK_GROUP_SIZE, &workGroupSize);
	printf("OpenCL workgroup size: %ld\n", workGroupSize);

	err = kernel->setArg(0, *outCL);
	checkErr(err, "Kernel::setArg()");

	queue = new cl::CommandQueue(*context, devices[0], 0, &err);
	checkErr(err, "CommandQueue::CommandQueue()");
	printf("OpenCL command queue prepared\n");

	ready = true;
}

void CclSupport::SetParams(sClParams ClParams, sClFractal ClFractal)
{
	cl_int err;
	err = kernel->setArg(1, ClParams);
	err = kernel->setArg(2, ClFractal);
	checkErr(err, "Kernel::setArg()");
}

void CclSupport::Render(void)
{
	cl_int err;
	float time_prev = clock();

	for (unsigned int loop = 0; loop < 1; loop++)
	{
		err = kernel->setArg(3, loop);

		cl::Event event;
		err = queue->enqueueNDRangeKernel(*kernel, cl::NullRange, cl::NDRange(CL_WIDTH * CL_HEIGHT), cl::NDRange(workGroupSize), NULL, &event);
		checkErr(err, "ComamndQueue::enqueueNDRangeKernel()");

		event.wait();
		err = queue->enqueueReadBuffer(*outCL, CL_TRUE, 0, buffSize, rgbbuff);
		checkErr(err, "ComamndQueue::enqueueReadBuffer()");
	}
	float time = clock();
	printf("time = %f \n", (time - time_prev) / CLOCKS_PER_SEC);
}

void CclSupport::Enable(void)
{
	if(ready)
	{
		enabled = true;
	}
}
