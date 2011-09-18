/*
 * cl_support.cpp
 *
 *  Created on: 18-09-2011
 *      Author: krzysztof marczak
 *      some small parts of code taken from: http://www.codeproject.com/KB/GPU-Programming/IntroToOpenCL.aspx
 */

#include "cl_support.hpp"

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

	std::ifstream file("cl_engine.cl");
	checkErr(file.is_open() ? CL_SUCCESS : -1, "cl_engine.cl");

	std::string prog(std::istreambuf_iterator<char>(file), (std::istreambuf_iterator<char>()));
	source = new cl::Program::Sources(1, std::make_pair(prog.c_str(), prog.length() + 1));
	printf("OpenCL kernel source loaded\n");

	program = new cl::Program(*context, *source);
	err = program->build(devices, "-w");
	checkErr(file.is_open() ? CL_SUCCESS : -1, "Program::build()");
	//std::cout << "Build options:\t" << program->getBuildInfo<CL_PROGRAM_BUILD_OPTIONS>(devices[0]) << std::endl;
	std::cout << "OpenCL Build log:\t" << program->getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[0]) << std::endl;
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
}

void CclSupport::SetParams(sClParams ClParams, sClFractal ClFractal)
{
	cl_int err;
	err = kernel->setArg(1, ClParams);
	err = kernel->setArg(2, ClFractal);
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
