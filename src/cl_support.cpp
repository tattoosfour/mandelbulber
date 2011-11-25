/*
 * cl_support.cpp
 *
 *  Created on: 18-09-2011
 *      Author: krzysztof marczak
 *      some small parts of code taken from: http://www.codeproject.com/KB/GPU-Programming/IntroToOpenCL.aspx
 */

#include <string>
#include "interface.h"
#include "settings.h"
#include "cl_support.hpp"

CclSupport *clSupport;

#ifdef CLSUPPORT
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
	width = 800;
	height = 600;
	steps = 10;
	recompileRequest = true;
	lastFormula = trig_optim;
}

void CclSupport::Init(void)
{
	char progressText[1000];
	sprintf(progressText, "OpenCL - initialization");
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(Interface.progressBar), progressText);
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(Interface.progressBar), 0.0);
	while (gtk_events_pending())
		gtk_main_iteration();

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

	std::string strFormula = "mandelbulb";

	if(lastFormula == trig_optim) strFormula = "mandelbulb";
	if(lastFormula == trig_DE) strFormula = "mandelbulb2";
	if(lastFormula == tglad) strFormula = "mandelbox";
	if(lastFormula == menger_sponge) strFormula = "mengersponge";
	if(lastFormula == hypercomplex) strFormula = "hypercomplex";
	if(lastFormula == kaleidoscopic) strFormula = "kaleidoscopic";

	std::string strFileEngine = clDir;
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkOpenClNoEffects)))
		strFileEngine += "cl_engine_fast.cl";
	else
		strFileEngine += "cl_engine.cl";
	std::ifstream fileEngine(strFileEngine.c_str());
	checkErr(fileEngine.is_open() ? CL_SUCCESS : -1, ("Can't open file:" + strFileEngine).c_str());

	std::string strFileDistance = clDir;
	if(lastFormula == hypercomplex)
		strFileDistance += "cl_distance_deltaDE.cl";
	else
		strFileDistance += "cl_distance.cl";
	std::ifstream fileDistance(strFileDistance.c_str());
	checkErr(fileDistance.is_open() ? CL_SUCCESS : -1, ("Can't open file:" + strFileDistance).c_str());

	std::string strFileFormulaBegin;
	if(lastFractal.juliaMode) strFileFormulaBegin  = clDir + "cl_formulaBegin" + "Julia" + ".cl";
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
	std::string progDistance(std::istreambuf_iterator<char>(fileDistance), (std::istreambuf_iterator<char>()));
	std::string progFormulaBegin(std::istreambuf_iterator<char>(fileFormulaBegin), (std::istreambuf_iterator<char>()));
	std::string progFormulaInit(std::istreambuf_iterator<char>(fileFormulaInit), (std::istreambuf_iterator<char>()));
	std::string progFormulaFor(std::istreambuf_iterator<char>(fileFormulaFor), (std::istreambuf_iterator<char>()));
	std::string progFormula(std::istreambuf_iterator<char>(fileFormula), (std::istreambuf_iterator<char>()));
	std::string progFormulaEnd(std::istreambuf_iterator<char>(fileFormulaEnd), (std::istreambuf_iterator<char>()));

	cl::Program::Sources sources;
	sources.push_back(std::make_pair(progEngine.c_str(), progEngine.length()));
	sources.push_back(std::make_pair(progDistance.c_str(), progDistance.length()));
	sources.push_back(std::make_pair(progFormulaBegin.c_str(), progFormulaBegin.length()));
	sources.push_back(std::make_pair(progFormulaInit.c_str(), progFormulaInit.length()));
	sources.push_back(std::make_pair(progFormulaFor.c_str(), progFormulaFor.length()));
	sources.push_back(std::make_pair(progFormula.c_str(), progFormula.length()));
	sources.push_back(std::make_pair(progFormulaEnd.c_str(), progFormulaEnd.length()+1));
	printf("OpenCL Number of loaded sources %d\n", sources.size());

	program = new cl::Program(*context, sources, &err);
	checkErr(err, "Program()");
	//program->getInfo(CL_PROGRAM_SOURCE, )
	std::cout << "Program source:\t" << program->getInfo<CL_PROGRAM_SOURCE>() << std::endl;

	chdir(clDir.c_str());

	err = program->build(devices, "-w");
	std::cout << "OpenCL Build log:\t" << program->getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[0]) << std::endl;
	checkErr(err, "Program::build()");
	printf("OpenCL program built done\n");

	chdir(data_directory);

	kernel = new cl::Kernel(*program, "fractal3D", &err);
	checkErr(err, "Kernel::Kernel()");
	printf("OpenCL kernel opened\n");

	kernel->getWorkGroupInfo(devices[0], CL_KERNEL_WORK_GROUP_SIZE, &workGroupSize);
	printf("OpenCL workgroup size: %ld\n", workGroupSize);

	steps = height * width / 50000 + 1;
	stepSize = (width * height / steps / workGroupSize + 1) * workGroupSize;
	buffSize = stepSize * sizeof(sClPixel);

	rgbbuff = new sClPixel[buffSize];
	outCL = new cl::Buffer(*context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR, buffSize,rgbbuff,&err);
	checkErr(err, "Buffer::Buffer()");
	printf("OpenCL buffer created\n");

	err = kernel->setArg(0, *outCL);
	checkErr(err, "Kernel::setArg()");

	queue = new cl::CommandQueue(*context, devices[0], 0, &err);
	checkErr(err, "CommandQueue::CommandQueue()");
	printf("OpenCL command queue prepared\n");

	sprintf(progressText, "OpenCL - ready");
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(Interface.progressBar), progressText);
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(Interface.progressBar), 1.0);

	char text[1000];
	sprintf(text,"OpenCL platform by: %s", platformVendor.c_str());
	gtk_label_set_text(GTK_LABEL(Interface.label_OpenClPlatformBy), text);
	sprintf(text,"GPU frequency: %d MHz", maxClockFrequency);
	gtk_label_set_text(GTK_LABEL(Interface.label_OpenClMaxClock), text);
	sprintf(text,"GPU memory: %ld MB", memorySize/1024/1024);
	gtk_label_set_text(GTK_LABEL(Interface.label_OpenClMemorySize), text);
	sprintf(text,"Number of computing units: %d", numberOfComputeUnits);
	gtk_label_set_text(GTK_LABEL(Interface.label_OpenClComputingUnits), text);
	sprintf(text,"Max workgroup size: %d", maxMaxWorkGroupSize[0]);
	gtk_label_set_text(GTK_LABEL(Interface.label_OpenClMaxWorkgroup), text);
	sprintf(text,"Actual workgroup size: %ld", workGroupSize);
	gtk_label_set_text(GTK_LABEL(Interface.label_OpenClWorkgroupSize), text);

	ready = true;
}

void CclSupport::SetParams(sClParams ClParams, sClFractal ClFractal, enumFractalFormula formula)
{
	if(ClFractal.juliaMode != lastFractal.juliaMode) recompileRequest = true;
	if(formula != lastFormula) recompileRequest = true;
	lastParams = ClParams;
	lastFractal = ClFractal;
	lastFormula = formula;
}

void CclSupport::Render(cImage *image, GtkWidget *outputDarea)
{
	cl_int err;

	if(recompileRequest)
	{
		Disable();
		Enable();
		Init();
		recompileRequest = false;
	}

	err = kernel->setArg(1, lastParams);
	err = kernel->setArg(2, lastFractal);
	checkErr(err, "Kernel::setArg()");

	for (unsigned int loop = 0; loop < steps; loop++)
	{
		err = kernel->setArg(3, stepSize * loop);

		cl::Event event;
		err = queue->enqueueNDRangeKernel(*kernel, cl::NullRange, cl::NDRange(stepSize), cl::NDRange(workGroupSize), NULL, &event);
		checkErr(err, "ComamndQueue::enqueueNDRangeKernel()");

		event.wait();
		err = queue->enqueueReadBuffer(*outCL, CL_TRUE, 0, buffSize, rgbbuff);
		checkErr(err, "ComamndQueue::enqueueReadBuffer()");

		unsigned int offset = loop * stepSize;
		for(unsigned int i=0; i<stepSize; i++)
		{
			unsigned int a = offset + i;
			sRGB16 pixel = {rgbbuff[i].R, rgbbuff[i].G, rgbbuff[i].B};
			int x = a % width;
			int y = a / width;
			image->PutPixelImage(x,y,pixel);
			image->PutPixelZBuffer(x,y,rgbbuff[i].zBuffer);
		}

		char progressText[1000];
		double percent = (double)loop/steps;
		sprintf(progressText, "OpenCL - rendering. Done %.1f%%", percent*100.0);
		gtk_progress_bar_set_text(GTK_PROGRESS_BAR(Interface.progressBar), progressText);
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(Interface.progressBar), percent);
		while (gtk_events_pending())
			gtk_main_iteration();
	}
	//gdk_draw_rgb_image(outputDarea->window, renderWindow.drawingArea->style->fg_gc[GTK_STATE_NORMAL], 0, 0, clSupport->GetWidth(), clSupport->GetHeight(), GDK_RGB_DITHER_NONE,
	//		clSupport->GetRgbBuff(), clSupport->GetWidth() * 3);
}

void CclSupport::Enable(void)
{
	if(ready)
	{
		enabled = true;
	}
}

void CclSupport::Disable(void)
{
	delete context;
	delete rgbbuff;
	delete outCL;
	delete program;
	delete kernel;
	delete queue;
	enabled = false;
	ready = false;
}

void CclSupport::SetSize(int w, int h)
{
	if(width != w || height != h) recompileRequest = true;
	width = w;
	height = h;
}

#endif
