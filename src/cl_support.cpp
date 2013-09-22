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
	lastEngineNumber = 0;
	lastStepSize = 0;
	inBuffer1 = new sClInBuff;
	constantsBuffer1 = new sClInConstants;
	auxReflectBuffer = NULL;
	buffSize = 0;
	context = NULL;
	kernel = NULL;
	outCL = NULL;
	rgbbuff = NULL;
	inCLBuffer1 = NULL;
	reflectBuffer = NULL;
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

	devices[0].getInfo(CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, &maxConstantBufferSize);
	printf("OpenCL Max constant buffer size  %ld kB\n", maxConstantBufferSize/1024);

	printf("OpenCL Constant buffer used  %ld kB\n", sizeof(sClInConstants)/1024);

	std::string clDir = std::string(sharedDir) + "cl/";

	std::string strFormula = "mandelbulb";

	if(lastFormula == trig_optim) strFormula = "mandelbulb";
	if(lastFormula == trig_DE) strFormula = "mandelbulb2";
	if(lastFormula == tglad) strFormula = "mandelbox";
	if(lastFormula == menger_sponge) strFormula = "mengersponge";
	if(lastFormula == hypercomplex) strFormula = "hypercomplex";
	if(lastFormula == quaternion) strFormula = "quaternion";
	if(lastFormula == kaleidoscopic) strFormula = "kaleidoscopic";
	if(lastFormula == xenodreambuie) strFormula = "xenodreambuie";

	std::string strFileEngine = clDir;
	int engineNumber = gtk_combo_box_get_active(GTK_COMBO_BOX(Interface.comboOpenCLEngine));
	if		 (engineNumber == 0) 	strFileEngine += "cl_engine_fast.cl";
	else if(engineNumber == 1)	strFileEngine += "cl_engine.cl";
	else if(engineNumber == 2)	strFileEngine += "cl_engine_full.cl";

	std::ifstream fileEngine(strFileEngine.c_str());
	checkErr(fileEngine.is_open() ? CL_SUCCESS : -1, ("Can't open file:" + strFileEngine).c_str());

	std::string strFileDistance = clDir;
	if(lastFormula == xenodreambuie || lastFormula == hypercomplex)
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
	printf("OpenCL Number of loaded sources %ld\n", sources.size());

	program = new cl::Program(*context, sources, &err);
	checkErr(err, "Program()");
	//program->getInfo(CL_PROGRAM_SOURCE, )
	//std::cout << "Program source:\t" << program->getInfo<CL_PROGRAM_SOURCE>() << std::endl;

	std::string buildParams;
	buildParams = "-w ";
	buildParams += "-I\"" + std::string(sharedDir) + "cl\" ";
	if(lastParams.DOFEnabled) buildParams += "-D_DOFEnabled ";
	if(lastParams.slowAmbientOcclusionEnabled) buildParams += "-D_SlowAOEnabled ";
	if(lastParams.auxLightNumber > 0) buildParams += "-D_AuxLightsEnabled ";
	err = program->build(devices, buildParams.c_str());
	std::cout << "OpenCL Build log:\t" << program->getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[0]) << std::endl;
	checkErr(err, "Program::build()");
	printf("OpenCL program built done\n");

	kernel = new cl::Kernel(*program, "fractal3D", &err);
	checkErr(err, "Kernel::Kernel()");
	printf("OpenCL kernel opened\n");

	kernel->getWorkGroupInfo(devices[0], CL_KERNEL_WORK_GROUP_SIZE, &workGroupSize);
	printf("OpenCL workgroup size: %ld\n", workGroupSize);


	int pixelsPerJob =  workGroupSize * numberOfComputeUnits;
	steps = height * width / pixelsPerJob + 1;
	stepSize = (width * height / steps / workGroupSize + 1) * workGroupSize;
	printf("OpenCL Job size: %d\n", stepSize);
	buffSize = stepSize * sizeof(sClPixel);
	rgbbuff = new sClPixel[buffSize];


	reflectBufferSize = sizeof(sClReflect) * 10 * stepSize;
	printf("reflectBuffer size = %d kB\n", reflectBufferSize / 1024);
	reflectBuffer = new sClReflect[reflectBufferSize];
	auxReflectBuffer = new cl::Buffer(*context, CL_MEM_READ_WRITE, reflectBufferSize, NULL, &err);

	inCLConstBuffer1 = new cl::Buffer(*context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, sizeof(sClInConstants), constantsBuffer1, &err);
	inCLBuffer1 = new cl::Buffer(*context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, sizeof(sClInBuff), inBuffer1, &err);

	outCL = new cl::Buffer(*context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR, buffSize,rgbbuff,&err);
	checkErr(err, "Buffer::Buffer()");
	printf("OpenCL buffers created\n");

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

void CclSupport::SetParams(sClInBuff *inBuff, sClInConstants *inConstants, enumFractalFormula formula)
{
	if(inConstants->fractal.juliaMode != lastFractal.juliaMode) recompileRequest = true;
	if(formula != lastFormula) recompileRequest = true;
	if(inConstants->params.DOFEnabled != lastParams.DOFEnabled) recompileRequest = true;
	if(inConstants->params.slowAmbientOcclusionEnabled != lastParams.slowAmbientOcclusionEnabled) recompileRequest = true;
	if(inConstants->params.auxLightNumber != lastParams.auxLightNumber) recompileRequest = true;

	int engineNumber = gtk_combo_box_get_active(GTK_COMBO_BOX(Interface.comboOpenCLEngine));
	if(engineNumber != lastEngineNumber) recompileRequest = true;
	lastEngineNumber = engineNumber;

	lastParams = inConstants->params;
	lastFractal = inConstants->fractal;
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

	stepSize = workGroupSize;
	int workGroupSizeMultiplier = 1;

	double startTime = real_clock();
	double lastTime = startTime;
	double lastTimeProcessing = startTime;
	double lastProcessingTime = 1.0;

	for (int pixelIndex = 0; pixelIndex < width * height; pixelIndex += stepSize)
	{

		delete outCL;
		delete[] rgbbuff;
		delete auxReflectBuffer;
		delete[] reflectBuffer;

		double processingCycleTime = atof(gtk_entry_get_text(GTK_ENTRY(Interface.edit_OpenCLProcessingCycleTime)));
		if(processingCycleTime < 0.1) processingCycleTime = 0.1;

		workGroupSizeMultiplier *= processingCycleTime / lastProcessingTime;

		int pixelsLeft = width * height - pixelIndex;
		int maxWorkGroupSizeMultiplier = pixelsLeft / workGroupSize;
		if(workGroupSizeMultiplier > maxWorkGroupSizeMultiplier) workGroupSizeMultiplier = maxWorkGroupSizeMultiplier;
		if(workGroupSizeMultiplier > 1024) workGroupSizeMultiplier = 1024;
		if(workGroupSizeMultiplier < numberOfComputeUnits) workGroupSizeMultiplier = numberOfComputeUnits;

		stepSize =  workGroupSize * workGroupSizeMultiplier;
		//printf("OpenCL Job size: %d\n", stepSize);
		buffSize = stepSize * sizeof(sClPixel);
		rgbbuff = new sClPixel[buffSize];
		outCL = new cl::Buffer(*context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR, buffSize,rgbbuff,&err);

		reflectBufferSize = sizeof(sClReflect) * 10 * stepSize;
		//printf("reflectBuffer size = %d\n", reflectBufferSize);
		reflectBuffer = new sClReflect[10 * stepSize];
		auxReflectBuffer = new cl::Buffer(*context, CL_MEM_READ_WRITE, reflectBufferSize, NULL, &err);

		checkErr(err, "Buffer::Buffer()");
		//printf("OpenCL buffers created\n");

		err = kernel->setArg(0, *outCL);
		err = kernel->setArg(1, *inCLBuffer1);
		err = kernel->setArg(2, *inCLConstBuffer1);
		err = kernel->setArg(3, *auxReflectBuffer);
		err = kernel->setArg(4, pixelIndex);

		//printf("size of inputs: %ld\n", sizeof(lastParams) + sizeof(lastFractal));

		err = queue->enqueueWriteBuffer(*inCLBuffer1, CL_TRUE, 0, sizeof(sClInBuff), inBuffer1);
		checkErr(err, "ComamndQueue::enqueueWriteBuffer()");
		err = queue->finish();
		checkErr(err, "ComamndQueue::finish() - CLBuffer");

		err = queue->enqueueWriteBuffer(*inCLConstBuffer1, CL_TRUE, 0, sizeof(sClInConstants), constantsBuffer1);
		checkErr(err, "ComamndQueue::enqueueWriteBuffer()");
		err = queue->finish();
		checkErr(err, "ComamndQueue::finish() - ConstBuffer");

		//cl::Event event;
		err = queue->enqueueNDRangeKernel(*kernel, cl::NullRange, cl::NDRange(stepSize), cl::NDRange(workGroupSize));//, NULL, &event);
		checkErr(err, "ComamndQueue::enqueueNDRangeKernel() ");
		//event.wait();
		err = queue->finish();
		checkErr(err, "ComamndQueue::finish() - Kernel");

		err = queue->enqueueReadBuffer(*outCL, CL_TRUE, 0, buffSize, rgbbuff);
		checkErr(err, "ComamndQueue::enqueueReadBuffer()");
		err = queue->finish();
		checkErr(err, "ComamndQueue::finish() - ReadBuffer");

		unsigned int offset = pixelIndex;

		for(unsigned int i=0; i<stepSize; i++)
		{
			unsigned int a = offset + i;
			sRGB16 pixel = {rgbbuff[i].R, rgbbuff[i].G, rgbbuff[i].B};
			int x = a % width;
			int y = a / width;
			image->PutPixelImage16(x,y,pixel);
			image->PutPixelZBuffer(x,y,rgbbuff[i].zBuffer);
		}

		char progressText[1000];
		double percent = (double)(pixelIndex + stepSize)/(width * height);
		if(percent > 1.0) percent = 1.0;
		double time = real_clock() - startTime;
		double time_to_go = (1.0 - percent) * time / percent;
		int togo_time_s = (int) time_to_go % 60;
		int togo_time_min = (int) (time_to_go / 60) % 60;
		int togo_time_h = time_to_go / 3600;
		int time_s = (int) time % 60;
		int time_min = (int) (time / 60) % 60;
		int time_h = time / 3600;
		sprintf(progressText, "OpenCL - rendering. Done %.1f%%, to go %dh%dm%ds, elapsed %dh%dm%ds, job size %d", percent*100.0, togo_time_h, togo_time_min, togo_time_s, time_h,
				time_min, time_s, stepSize);
		gtk_progress_bar_set_text(GTK_PROGRESS_BAR(Interface.progressBar), progressText);
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(Interface.progressBar), percent);

		lastProcessingTime = time - lastTimeProcessing;
		lastTimeProcessing = time;

		if (real_clock() - lastTime > 30.0)
		{
			if (image->IsPreview())
			{
				image->ConvertTo8bit();
				image->UpdatePreview();
				image->RedrawInWidget(outputDarea);
				while (gtk_events_pending())
					gtk_main_iteration();
			}
			lastTime = real_clock();
		}

		if(programClosed) break;

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
	delete[] rgbbuff;
	delete outCL;
	delete program;
	delete kernel;
	delete queue;
	delete inCLBuffer1;
	delete inCLConstBuffer1;
	delete[] reflectBuffer;
	enabled = false;
	ready = false;
}

void CclSupport::SetSize(int w, int h)
{
	//if(width != w || height != h) recompileRequest = true;
	width = w;
	height = h;
}

#endif
