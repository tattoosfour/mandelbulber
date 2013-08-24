/*********************************************************
 /                   MANDELBULBER                        *
 /                                                       *
 / author: Krzysztof Marczak                             *
 / contact: buddhi1980@gmail.com                         *
 / licence: GNU GPL v3.0                                 *
 /                                                       *
 / Windows port by Makemeunsee, Knigty                   *
 ********************************************************/

#ifdef WIN32 /* WINDOWS */
#include <windows.h>
#define HAVE_BOOLEAN  /* prevent jmorecfg.h from redefining it */
#endif

#ifndef WIN32
#include <sys/time.h>
#endif

#include <cstdio>
#include <string.h>
#include <string>
#include <cstdlib>
#include <unistd.h>
#include <locale.h>
#include <dirent.h>
#include <sys/stat.h>

#include "Render3D.h"
#include "files.h"
#include "shaders.h"
#include "interface.h"
#include "timeline.hpp"
#include "morph.hpp"
#include "settings.h"
#include "undo.hpp"
#include "callbacks.h"
#include "netrender.hpp"

using namespace std;

bool noGUI = false;

int NR_THREADS;

JSAMPLE *tekstura;
JSAMPLE *lightMap;

//global conters
guint64 N_counter = 0;
guint64 Loop_counter = 0;
guint64 DE_counter = 0;
guint64 DE_counterForDEerror;
guint64 Pixel_counter = 0;
int Missed_DE_counter = 0;
double DEerror = 0;

double start_time = 0.0;
bool isRendering = false;
bool isPostRendering = false;

bool newLineRendered = false;

cImage mainImage(800, 600);

double real_clock(void)
{
#ifdef WIN32 /* WINDOWS */
	return (double) clock() / CLOCKS_PER_SEC;
#else               /*other unix - try sysconf*/
	struct timeval tv;
	gettimeofday(&tv, 0);
	return tv.tv_sec + 1e-6 * tv.tv_usec;
#endif  /* WINDOWS */
}


CVector3 RayMarching(sParamRender *param, sFractal *calcParam, CVector3 start, CVector3 direction, double maxScan, bool binaryEnable, sStep *stepBuff, int *buffCount,
		double *distThreshOut, double *lastDistOut, bool *foundOut, double *depthOut)
{
	CVector3 point;
	bool found = false;
	double scan = param->doubles.viewDistanceMin;
	double distThresh = *distThreshOut;
	double dist = 0;
	double search_accuracy = 0.01 * param->doubles.quality;
	double search_limit = 1.0 - search_accuracy;
	int counter = 0;
	double step = param->doubles.viewDistanceMin;
	*buffCount = 0;
	double distThreshInit = *distThreshOut;

	for (int i = 0; i < 10000; i++)
	{
		counter++;

		point = start + direction * scan;
		bool max_iter = false;

		if (calcParam->constantDEThreshold)
		{
			distThresh = param->doubles.quality;
		}
		else
		{
			distThresh = scan * param->doubles.resolution * param->doubles.persp / param->doubles.quality + distThreshInit;
		}
		if(param->perspectiveType == equirectangular || param->perspectiveType == fishEye) distThresh *= M_PI;
		calcParam->doubles.detailSize = distThresh;
		dist = CalculateDistance(point, *calcParam, &max_iter);

		//printf("Distance = %g\n", dist/distThresh);
		stepBuff[i].distance = dist;
		stepBuff[i].iters = calcParam->itersOut;
		stepBuff[i].distThresh = distThresh;

		if (dist > 3.0) dist = 3.0;
		if (dist < distThresh)
		{
			if (dist < 0.1 * distThresh) Missed_DE_counter++;
			found = true;
			break;
		}

		stepBuff[i].step = step;
		step = (dist - 0.5 * distThresh) * param->doubles.DE_factor * (1.0 - Random(1000)/10000.0);;
		stepBuff[i].point = point;
		(*buffCount)++;
		scan += step;
		if (scan > maxScan)
		{
			break;
		}

	}

	if (found && binaryEnable)
	{
		double step = distThresh * 0.5;
		for (int i = 0; i < 10; i++)
		{
			DE_counter++;
			if (dist < distThresh && dist > distThresh * search_limit)
			{
				break;
			}
			else
			{
				if (dist > distThresh)
				{
					scan += step;
					point = start + direction * scan;
				}
				else if (dist < distThresh * search_limit)
				{
					scan -= step;
					point = start + direction * scan;
				}
			}
			bool max_iter = false;
			dist = CalculateDistance(point, *calcParam, &max_iter);
			//printf("Distance binary = %g\n", dist/distThresh);
			step *= 0.5;
		}
	}

	DE_counter += counter;
	Pixel_counter++;
	//counters for drawing histogram
	int counter2 = counter / 4;
	if (counter2 < 256) histogram2[counter2]++;
	else histogram2[255]++;

	*foundOut = found;
	*distThreshOut = distThresh;
	*lastDistOut = dist;
	*depthOut = scan;
	return point;
}

//***************************** Main thread ****************************
//main rendering thread
void *MainThread(void *ptr)
{

	//getting parameters
	sParam *threadParameters;
	threadParameters = (sParam*) ptr;

	//initialising variables
	int yScrStart = threadParameters->start;
	yScrStart = (yScrStart / 16) * 16;
	int yScr = threadParameters->z;
	int thread_number = yScr;
	cImage *image = threadParameters->image;
	int progressive = threadParameters->progressive;
	int progressiveStart = threadParameters->progressiveStart;
	int width = image->GetWidth();
	int height = image->GetHeight();
	WriteLogDouble("Thread started", thread_number);

	//printf("Thread #%d started\n", z + 1);
	sParamRender param = threadParameters->param;

	int tiles = param.noOfTiles;
	int tile = param.tileCount;
	CVector3 vp = param.doubles.vp;
	double maxDepth = param.doubles.viewDistanceMax;
	double zoom = param.doubles.zoom;
	double alpha = param.doubles.alpha;
	double beta = param.doubles.beta;
	double gamma = param.doubles.gamma;
	int AO_quality = param.globalIlumQuality;
	cTexture *lightmap_texture = param.lightmapTexture;

	//preparing rotation matrix
	CRotationMatrix mRot;
	mRot.RotateZ(alpha);
	mRot.RotateX(beta);
	mRot.RotateY(gamma);

	//preparing base vectors
	CVector3 vector, shadowVector;
	CVector3 baseX(1.0, 0.0, 0.0);
	CVector3 baseY(0.0, 1.0, 0.0);
	CVector3 baseZ(0.0, 0.0, 1.0);

	baseX = mRot.RotateVector(baseX);
	baseY = mRot.RotateVector(baseY);
	baseZ = mRot.RotateVector(baseZ);

	//shadow direction vector
	vector.x = cos(param.doubles.mainLightAlpha - 0.5 * M_PI) * cos(-param.doubles.mainLightBeta);
	vector.y = sin(param.doubles.mainLightAlpha - 0.5 * M_PI) * cos(-param.doubles.mainLightBeta);
	vector.z = sin(-param.doubles.mainLightBeta);
	shadowVector = mRot.RotateVector(vector);


	//reflection data
	int maxRay = param.reflectionsMax;
	if (!param.imageSwitches.raytracedReflections) maxRay = 0;
	sReflect *reflectBuff = new sReflect[maxRay + 2];

	for(int i = 0; i < maxRay + 1; i++)
	{
		//rayMarching buffers
		reflectBuff[i].stepBuff = new sStep[10002];
		reflectBuff[i].buffCount = 0;
	}

	//calculating vectors for AmbientOcclusion
	sVectorsAround *vectorsAround = new sVectorsAround[10000];
	int vectorsCount;
	int counter = 0;
	int lightMapWidth = param.lightmapTexture->Width();
	int lightMapHeight = param.lightmapTexture->Height();
	for (double b = -0.49 * M_PI; b < 0.49 * M_PI; b += 1.0 / AO_quality)
	{
		for (double a = 0.0; a < 2.0 * M_PI; a += ((2.0 / AO_quality) / cos(b)))
		{
			CVector3 d;
			d.x = cos(a + b) * cos(b);
			d.y = sin(a + b) * cos(b);
			d.z = sin(b);
			vectorsAround[counter].alpha = a;
			vectorsAround[counter].beta = b;
			vectorsAround[counter].v = d;
			int X = (int) ((a + b) / (2.0 * M_PI) * lightMapWidth + lightMapWidth * 8.5) % lightMapWidth;
			int Y = (int) (b / (M_PI) * lightMapHeight + lightMapHeight * 8.5) % lightMapHeight;
			vectorsAround[counter].R = lightmap_texture->FastPixel(X, Y).R;
			vectorsAround[counter].G = lightmap_texture->FastPixel(X, Y).G;
			vectorsAround[counter].B = lightmap_texture->FastPixel(X, Y).B;
			if (vectorsAround[counter].R > 10 || vectorsAround[counter].G > 10 || vectorsAround[counter].B > 10)
			{
				counter++;
			}
			if (counter >= 10000) break;
		}
		if (counter >= 10000) break;
	}
	if (counter == 0)
	{
		counter = 1;
		vectorsAround[0].alpha = 0;
		vectorsAround[0].beta = 0;
		vectorsAround[0].v.x = 0;
		vectorsAround[0].v.y = 0;
		vectorsAround[0].v.z = 0;
		vectorsAround[0].R = 0;
		vectorsAround[0].G = 0;
		vectorsAround[0].B = 0;
	}
	vectorsCount = counter;
	//printf("vectors count %d\n", counter);

	//aspect ratio
	double aspectRatio = (double) width / height;

	//starting point for raymarching

	enumPerspectiveType perspectiveType = param.perspectiveType;
	double fov = param.doubles.persp;

	//raymarching start point
	CVector3 start;
	if (perspectiveType == fishEye || perspectiveType == equirectangular)
	{
		start = vp + baseY * param.doubles.viewDistanceMin;
	}
	else
	{
		start = vp - baseY * (1.0 / fov * zoom + param.doubles.viewDistanceMin);
	}

	//parameters for iteration functions
	sFractal calcParam = param.fractal;

	WriteLogDouble("All vectors and matrices prepared", thread_number);

	double tileXOffset = (tile % tiles);
	double tileYOffset = (tile / tiles);

	bool breakX = false;

	//2-pass loop (for multi-threading)
	for (int pass = 0; pass < 3; pass++)
	{
		if (pass > 0) yScrStart = 0;

		//main loop for z values
		for (yScr = yScrStart; yScr <= height - progressive; yScr += progressive)
		{
			//checking if some another thread is not rendering the same z
			if (threadParameters->thread_done[yScr] == 0 && !breakX)
			{
				//giving information for another threads that this thread renders this z value
				threadParameters->thread_done[yScr] = thread_number + 1;

				newLineRendered = true;

				//WriteLogDouble("Started rendering line", z);

				//main loop for x values
				breakX = false;
				for (int xScr = 0; xScr <= width - progressive; xScr += progressive)
				{
					//checking if program was not closed
					if (programClosed)
					{
						delete[] vectorsAround;
						for(int i = 0; i < maxRay + 1; i++)
						{
							delete[] reflectBuff[i].stepBuff;
						}
						delete[] reflectBuff;
						return NULL;
					}

					//recheck threads
					if (threadParameters->thread_done[yScr] == thread_number + 1 || threadParameters->thread_done[yScr] == 0)
					{

						if (progressive < progressiveStart && xScr % (progressive * 2) == 0 && yScr % (progressive * 2) == 0) continue;

						//------------- finding fractal surface ---------------------------

						//calculating of viewVector
						CVector3 viewVector;
						double x2 = ((double) xScr / width / tiles - 0.5 + tileXOffset / tiles) * aspectRatio;
						double z2 = ((double) yScr / height / tiles - 0.5 + tileYOffset / tiles);

						bool hemisphereCut = false;
						if (perspectiveType == fishEye)
						{
							if (param.fishEyeCut && sqrt(x2 * x2 + z2 * z2) > 0.5 / fov)
							{
								hemisphereCut = true;
							}

							double x3 = x2 * M_PI;
							double z3 = z2 * M_PI;

							double r = sqrt(x3 * x3 + z3 * z3);

							if(r == 0)
							{
								viewVector.x = 0.0;
								viewVector.z = 0.0;
								viewVector.y = 1.0;
							}
							else
							{
								viewVector.x = x3 / r * sin(r * fov);
								viewVector.z = z3 / r * sin(r * fov);
								viewVector.y = cos(r * fov);
							}
						}
						else if(perspectiveType == equirectangular)
						{
							double x3 = x2 * M_PI;
							double z3 = z2 * M_PI;

							viewVector.x = sin(fov * x3) * cos(fov * z3);
							viewVector.z = sin(fov * z3);
							viewVector.y = cos(fov * x3) * cos(fov * z3);
						}
						else //3-point perspective
						{
							viewVector.x = x2 * fov;
							viewVector.y = 1.0;
							viewVector.z = z2 * fov;
						}
						viewVector.Normalize();
						viewVector = mRot.RotateVector(viewVector);

						//Ray marching

						double distThresh = 0.0;

						CVector3 point;
						CVector3 startRay = start;

						sShaderOutput resultShader = { 0.0, 0.0, 0.0 };
						sShaderOutput objectColour = { 0.0, 0.0, 0.0 };

						int rayEnd = 0;
						for (int ray = 0; ray <= maxRay; ray++)
						{
							reflectBuff[ray].start = startRay;
							reflectBuff[ray].viewVector = viewVector;

							reflectBuff[ray].found = false;
							reflectBuff[ray].buffCount = 0;

							if (!hemisphereCut)
							{
								point = RayMarching(&param, &calcParam, startRay, viewVector, maxDepth, true, reflectBuff[ray].stepBuff, &reflectBuff[ray].buffCount, &distThresh,
										&reflectBuff[ray].lastDist, &reflectBuff[ray].found, &reflectBuff[ray].depth);
							}
							else
							{
								reflectBuff[ray].found = false;
							}
							reflectBuff[ray].point = point;
							reflectBuff[ray].distThresh = distThresh;
							reflectBuff[ray].objectType = calcParam.objectOut;

							if(calcParam.objectOut == objFractal) reflectBuff[ray].reflect = param.doubles.imageAdjustments.reflect;
							if(calcParam.objectOut == objWater) reflectBuff[ray].reflect = param.doubles.primitiveWaterReflect;
							if(calcParam.objectOut == objPlane) reflectBuff[ray].reflect = param.doubles.primitivePlaneReflect;
							if(calcParam.objectOut == objBox) reflectBuff[ray].reflect = param.doubles.primitiveBoxReflect;
							if(calcParam.objectOut == objBoxInv) reflectBuff[ray].reflect = param.doubles.primitiveInvertedBoxReflect;
							if(calcParam.objectOut == objSphere) reflectBuff[ray].reflect = param.doubles.primitiveSphereReflect;
							if(calcParam.objectOut == objSphereInv) reflectBuff[ray].reflect = param.doubles.primitiveInvertedSphereReflect;

							rayEnd = ray;
							if(!reflectBuff[ray].found) break;
							if(reflectBuff[ray].reflect == 0) break;

							//calculate new ray direction and start point
							startRay = point;
							sShaderInputData shaderInputData;
							shaderInputData.calcParam = &calcParam;
							shaderInputData.param = &param;
							shaderInputData.dist_thresh = reflectBuff[ray].distThresh;
							shaderInputData.lightVect = shadowVector;
							shaderInputData.point = point;
							shaderInputData.viewVector = viewVector;
							if(param.fractal.constantDEThreshold)	shaderInputData.delta = reflectBuff[ray].depth * param.doubles.resolution * param.doubles.persp;
							else shaderInputData.delta = reflectBuff[ray].distThresh * param.doubles.quality;
							CVector3 vn = CalculateNormals(shaderInputData);
							viewVector = viewVector - vn * viewVector.Dot(vn)*2.0;
							startRay = startRay + viewVector * reflectBuff[ray].distThresh;
						}

						for(int ray = rayEnd; ray >= 0; ray--)
						{

							sShaderOutput objectShader = { 0.0, 0.0, 0.0, 0.0 };
							sShaderOutput backgroudShader = { 0.0, 0.0, 0.0, 0.0 };
							sShaderOutput volumetricShader  = { 0.0, 0.0, 0.0, 0.0 };
							sShaderOutput specular  = { 0.0, 0.0, 0.0, 0.0 };

							CVector3 lightVector = shadowVector;

							sShaderInputData shaderInputData;
							shaderInputData.calcParam = &calcParam;
							shaderInputData.param = &param;
							shaderInputData.dist_thresh = reflectBuff[ray].distThresh;
							if(param.fractal.constantDEThreshold)	shaderInputData.delta = reflectBuff[ray].depth * param.doubles.resolution * param.doubles.persp;
							else shaderInputData.delta = reflectBuff[ray].distThresh * param.doubles.quality;
							shaderInputData.calcParam->doubles.detailSize = reflectBuff[ray].distThresh;
							shaderInputData.lightVect = lightVector;
							shaderInputData.point = reflectBuff[ray].point;
							shaderInputData.viewVector = reflectBuff[ray].viewVector;
							shaderInputData.vectorsCount = vectorsCount;
							shaderInputData.vectorsAround = vectorsAround;
							shaderInputData.lastDist = reflectBuff[ray].lastDist;
							shaderInputData.envMappingTexture = param.envmapTexture;
							shaderInputData.depth = reflectBuff[ray].depth;
							shaderInputData.stepCount = reflectBuff[ray].buffCount;
							shaderInputData.stepBuff = reflectBuff[ray].stepBuff;
							shaderInputData.objectType = reflectBuff[ray].objectType;

							//if fractal surface was found
							if (reflectBuff[ray].found)
							{
								objectShader = ObjectShader(shaderInputData, &objectColour, &specular);

							} //end if found
							else
							{
								backgroudShader = BackgroundShader(shaderInputData);
								reflectBuff[ray].depth = 1e20;
							}

							double reflect = reflectBuff[ray].reflect;

							sShaderOutput pixel;

							if (maxRay > 0 && rayEnd > 0 && ray != rayEnd)
							{

								pixel.R = resultShader.R * reflect + (1.0 - reflect) * (objectShader.R + backgroudShader.R) + specular.R;
								pixel.G = resultShader.G * reflect + (1.0 - reflect) * (objectShader.G + backgroudShader.G) + specular.G;
								pixel.B = resultShader.B * reflect + (1.0 - reflect) * (objectShader.B + backgroudShader.B) + specular.B;

							}
							else
							{
								pixel.R = objectShader.R + backgroudShader.R + specular.R;
								pixel.G = objectShader.G + backgroudShader.G + specular.G;
								pixel.B = objectShader.B + backgroudShader.B + specular.B;
							}

							pixel.alpha = objectShader.alpha + backgroudShader.alpha;
							sShaderOutput opacityOut;
							volumetricShader = VolumetricShader(shaderInputData, pixel, &opacityOut);
							resultShader.R = volumetricShader.R;
							resultShader.G = volumetricShader.G;
							resultShader.B = volumetricShader.B;

							if(ray == 0)
							{
								image->PutPixelOpacity(xScr, yScr, (unsigned short)(opacityOut.R * 65535.0));
								image->PutPixelAlpha(xScr, yScr, (unsigned short)(volumetricShader.alpha * 65535.0));
							}

						}

						sRGBfloat pixel2;
						pixel2.R = resultShader.R;
						pixel2.G = resultShader.G;
						pixel2.B = resultShader.B;

						sRGB8 colour;
						colour.R = objectColour.R * 255;
						colour.G = objectColour.G * 255;
						colour.B = objectColour.B * 255;

						image->PutPixelImage(xScr, yScr, pixel2);
						image->PutPixelColour(xScr, yScr, colour);
						image->PutPixelZBuffer(xScr, yScr, (float)reflectBuff[0].depth);
					}

					else
					{
						breakX = true;
						break;
					}

				}					//next x

				if (!breakX) threadParameters->linesToSend[yScr] = true;

				image->Squares(yScr, progressive);

				(*threadParameters->done)++;

				double progressiveDone, percent_done;
				if (progressive == progressiveStart) progressiveDone = 0;
				else progressiveDone = 0.25 / (progressive * progressive);

				if (progressiveStart == 1)
				{
					percent_done = ((double) *threadParameters->done / height) * 100.0;
				}
				else
				{
					percent_done = (((double) *threadParameters->done / height) * 3.0 / 4.0 / progressive + progressiveDone) * 100.0;
				}

				if (param.noOfTiles > 1)
				{
					percent_done = (param.tileCount + percent_done / 100.0) / (param.noOfTiles * param.noOfTiles) * 100.0;
				}
				if (Interface_data.animMode)
				{
					percent_done = (param.fractal.frameNo - param.startFrame + percent_done / 100.0) / (param.endFrame - param.startFrame + 1) * 100.0;
				}

				double time = real_clock() - start_time;
				double time_to_go = (100.0 - percent_done) * time / percent_done;
				int togo_time_s = (int) time_to_go % 60;
				int togo_time_min = (int) (time_to_go / 60) % 60;
				int togo_time_h = time_to_go / 3600;

				int time_s = (int) time % 60;
				int time_min = (int) (time / 60) % 60;
				int time_h = time / 3600;

				double iterations_per_sec = N_counter / time;
				if (!param.quiet)
				{
					printf("Done %.3f%%, to go = %dh%dm%ds, elapsed = %dh%dm%ds, iters/s = %.0f       \r", percent_done, togo_time_h, togo_time_min, togo_time_s, time_h, time_min, time_s,
							iterations_per_sec);
					fflush(stdout);
				}
				//printing to console some statistics
				if (*threadParameters->done == height - 1)
				{
					double avg_N = (double) N_counter / Loop_counter;
					double avg_DE = (double) DE_counter / Pixel_counter;
					double avgMissedDE = (double) Missed_DE_counter / Pixel_counter * 100.0;
					printf("Average N = %f, Average DE steps = %f, Missed DE %.3f%%\n", avg_N, avg_DE, avgMissedDE);
				}

				//WriteLogDouble("Rendering line finished", z);
			}					//end if thread done
			else
			{
				breakX = false;
				if (pass < 2)
				{
					int firstFree = -1;
					int lastFree = -1;
					int maxHole = 0;
					for (int i = 0; i < height; i += progressive)
					{
						if (firstFree < 0 && threadParameters->thread_done[i] == 0)
						{
							firstFree = i;
						}
						if (firstFree > 0 && threadParameters->thread_done[i] > 0)
						{
							lastFree = i;

							int holeSize = lastFree - firstFree;
							if (holeSize > maxHole)
							{
								yScr = (((firstFree + lastFree) / 2) / progressive) * progressive - progressive;
								maxHole = holeSize;
							}
							firstFree = -1;
							lastFree = -1;
						}
					}
					if (firstFree > 0 && lastFree < 0)
					{
						yScr = (((firstFree + height) / 2) / progressive) * progressive;
						maxHole = 1;
					}
					if (maxHole == 0) break;
				}

				if (programClosed) break;
			}
		}					//next z
	}					// next pass
	WriteLogDouble("Thread finished", thread_number);
	delete[] vectorsAround;

	for(int i = 0; i < maxRay + 1; i++)
	{
		delete[] reflectBuff[i].stepBuff;
	}
	delete[] reflectBuff;

	return 0;
}

//************************** Render **************************************
void Render(sParamRender param, cImage *image, GtkWidget *outputDarea)
{

	programClosed = false;
	//getting image resolution from parameters
	CheckPrameters(&param);

	int width = image->GetWidth();
	int height = image->GetHeight();

	if (!clSupport->IsReady())
	{
		//turn off refreshing if resolution is very low
		bool bo_refreshing = true;

		//sending settings to clients
		if(netRender->IsServer()) SendSettingsToClients(param);

		//if it is a server, send "run" command to all clients
		if (netRender->IsServer())
		{
			for (int i = 0; i < netRender->getNoOfClients(); i++)
			{
				netRender->sendDataToClient(NULL, 0, (char*)"run", i, param.fractal.frameNo);
			}
		}

		//calculate total number of vailable CPUs
		int netCpuCount = NR_THREADS;
		for(int i=0; i<netRender->getNoOfClients(); i++)
		{
			netCpuCount += netRender->getCpuCount(i);
		}

		//setting first lines to render
		int cpuIndex = NR_THREADS;
		if (netRender->IsServer())
		{
			for (int clientIndex = 0; clientIndex < netRender->getNoOfClients(); clientIndex++)
			{
				for(int i=0; i<netRender->getCpuCount(clientIndex); i++)
				{
					int startLine = cpuIndex * height / netCpuCount;
					netRender->sendDataToClient(&startLine, sizeof(startLine), (char*)"stl", clientIndex, 0);
					printf("set start line = %d\n", startLine);
					cpuIndex++;
				}
			}
		}

		//clearing histogram tables
		for (int i = 0; i < 64; i++)
			histogram[i] = 0;
		for (int i = 0; i < 256; i++)
			histogram2[i] = 0;
		WriteLog("Histogram data cleared");

		//reseting counters
		N_counter = 0;
		Loop_counter = 0;
		DE_counter = 0;
		Pixel_counter = 0;
		Missed_DE_counter = 0;
		DEerror = 0;
		DE_counterForDEerror = 0;

		//initialising threads
		int *thread_done = new int[height];
		int *thread_done_from_server = new int[height];
		bool *linesToSend = new bool[height];
		GThread **Thread = new GThread *[NR_THREADS + 1];
		GError **err = new GError *[NR_THREADS + 1];
		sParam *thread_param = new sParam[NR_THREADS];

		//initialise send/receive buffer
		sAllImageData *lineOfImage = NULL;
		lineOfImage = new sAllImageData[width+1];

		int progressiveStart = 8;
		if (param.recordMode || noGUI || netRender->IsServer() || netRender->IsClient()) progressiveStart = 1;
		image->progressiveFactor = progressiveStart;

		double refresh_skip = 0.1;
		double refreshEndTime = real_clock();

		image->CalculateGammaTable();

		for (int progressive = progressiveStart; progressive != 0; progressive /= 2)
		{
			//if (width * height / progressive / progressive <= 100 * 100) bo_refreshing = false;
			//else bo_refreshing = true;

			int done = 0;
			for (int i = 0; i < NR_THREADS; i++)
			{
				err[i] = NULL;
			}
			for (int i = 0; i < height; i++)
			{
				thread_done[i] = 0;
				thread_done_from_server[i] = 0;
				linesToSend[i] = false;
			}
			WriteLog("Thread data prepared");

			//running rendering threads in background
			for (int i = 0; i < NR_THREADS; i++)
			{
				//sending some parameters to thread
				if(!netRender->IsClient())
				{
					thread_param[i].start = i * height / netCpuCount;
				}
				else
				{
					//getting indexes of first lines to render
					int startLine = 0;
					char command[4];
					size_t bytes_received = netRender->receiveDataFromServer(command);
					if(!strcmp(command, "stl") && bytes_received == sizeof(int))
					{
						netRender->GetData(&startLine);
					}
					printf("command = %s, start line = %d\n", command, startLine);
					thread_param[i].start = startLine;
				}

				//initialising local threads
				thread_param[i].z = i;
				thread_param[i].param = param;
				thread_param[i].image = image;
				thread_param[i].progressive = progressive;
				thread_param[i].progressiveStart = progressiveStart;
				thread_param[i].done = &done;
				thread_param[i].thread_done = thread_done;
				thread_param[i].linesToSend = linesToSend;
				//creating thread
				Thread[i] = g_thread_create((GThreadFunc) MainThread, &thread_param[i], TRUE, &err[i]);
				WriteLogDouble("Thread created", i);
				//printf("Rendering thread #%d created\n", i + 1);
			}

			//refresh GUI
			if ((!noGUI && bo_refreshing) || netRender->IsClient())
			{
				if (image->IsPreview() || netRender->IsClient())
				{
					while (gtk_events_pending())
						gtk_main_iteration();
				}

				int done2 = 0;
				//refreshing image and histograms during rendering
				while (done2 < height / progressive - 1 && !programClosed)
				{
					//send list of rendered lines
					if (netRender->IsServer() && newLineRendered)
					{
						newLineRendered = false;
						for (int i = 0; i < netRender->getNoOfClients(); i++)
						{
							netRender->sendDataToClient(thread_done, sizeof(int)*height, (char*)"lst", i, param.fractal.frameNo);
						}
					}

					char command[4];
					//get rendered line if available
					if (netRender->IsServer())
					{
						for (int i = 0; i < netRender->getNoOfClients(); i++)
						{
							netRender->sendDataToClient(NULL, 0, (char*)"get", i, param.fractal.frameNo);
						}

						for (int i = 0; i < netRender->getNoOfClients(); i++)
						{
							size_t bytes_recvd = netRender->receiveDataFromClient(command, i, param.fractal.frameNo);
							if(!strcmp(command, "GET"))
							{
								if(bytes_recvd > 0)
								{
									int *last;
									netRender->GetData(lineOfImage);
									last = (int*)&lineOfImage[width];

									int y = *last;
									if(y<height)
									{
										for(int x=0; x<width; x++)
										{
												image->PutPixelImage(x,y, lineOfImage[x].imageFloat);
												image->PutPixelAlpha(x,y, lineOfImage[x].alphaBuffer);
												image->PutPixelZBuffer(x,y, lineOfImage[x].zBuffer);
												image->PutPixelColour(x,y, lineOfImage[x].colourBuffer);
												image->PutPixelOpacity(x,y, lineOfImage[x].opacityBuffer);
										}
										thread_done[y] = 99;
										newLineRendered = true;
									}
								}
							}
						}
					}

					if (netRender->IsClient())
					{
						//receiving list of already rendered lines
						netRender->receiveDataFromServer(command);
						if(!strcmp(command, "lst"))
						{
							netRender->GetData(thread_done_from_server);
							memcpy(thread_done, thread_done_from_server, sizeof(int)*height);
						}

						//sending rendered line
						if(!strcmp(command, "get"))
						{
							bool sentSomething = false;
							for(int y=0; y<height; y++)
							{
								if(linesToSend[y] && thread_done_from_server[y] == 0)
								{
									linesToSend[y] = false;
									for(int x=0; x<width; x++)
									{
											lineOfImage[x].imageFloat = image->GetPixelImage(x, y);
											lineOfImage[x].alphaBuffer = image->GetPixelAlpha(x, y);
											lineOfImage[x].colourBuffer = image->GetPixelColor(x, y);
											lineOfImage[x].zBuffer = image->GetPixelZBuffer(x, y);
											lineOfImage[x].opacityBuffer = image->GetPixelOpacity(x, y);
									}
									int *last;

									last = (int*)&lineOfImage[width];
									*last = y;

									netRender->sendDataToServer(lineOfImage, sizeof(sAllImageData)*(width+1),(char*)"GET");

									sentSomething = true;
									break;
								}
							}
							if(!sentSomething)
							{
								//sending null answer when there si nothing to send
								netRender->sendDataToServer(NULL,0,(char*)"GET");
							}
						}

						if(!strcmp(command, "stp"))
						{
							programClosed = true;
						}
					}

					if(!netRender->IsClient() && !netRender->IsServer()) g_usleep(100000);


					if (!noGUI)
					{
						if (progressive < progressiveStart || netRender->IsServer() || netRender->IsClient())
						{

							if (real_clock() - refreshEndTime >= refresh_skip)
							{
								double refreshStartTime = real_clock();

								if (image->IsPreview())
								{
									image->CompileImage();
									WriteLog("Image compiled");

									image->ConvertTo8bit();
									WriteLog("Image converted to 8-bit");
									image->UpdatePreview();
									WriteLog("Preview created");
									if (outputDarea != NULL)
									{
										DrawHistogram();
										DrawHistogram2();
										WriteLog("Histograms refreshed");
										image->RedrawInWidget(outputDarea);
										WriteLog("Image redrawn in window");
									}
								}

								refreshEndTime = real_clock();
								//printf("RefreshTime: %f\n", refreshEndTime - refreshStartTime);
								refresh_skip = (refreshEndTime - refreshStartTime) * 10;
								WriteLogDouble("Image refreshed", refreshEndTime - refreshStartTime);
							}
						}
					}

					if (!noGUI)
					{
						if (outputDarea != NULL)
						{
							//progress bar
							char progressText[1000];
							double progressiveDone, percent_done;
							if (progressive == progressiveStart) progressiveDone = 0;
							else progressiveDone = 0.25 / (progressive * progressive);
							if (progressiveStart == 1)
							{
								percent_done = ((double) done2 / height) * 100.0;
							}
							else
							{
								percent_done = (((double) done2 / height) * 3.0 / 4.0 / progressive + progressiveDone) * 100.0;
							}

							if (param.noOfTiles > 1)
							{
								percent_done = (param.tileCount + percent_done / 100.0) / (param.noOfTiles * param.noOfTiles) * 100.0;
							}
							if (Interface_data.animMode)
							{
								percent_done = (param.fractal.frameNo - param.startFrame + percent_done / 100.0) / (param.endFrame - param.startFrame + 1) * 100.0;
							}

							double time = real_clock() - start_time;
							double time_to_go = (100.0 - percent_done) * time / percent_done;
							int togo_time_s = (int) time_to_go % 60;
							int togo_time_min = (int) (time_to_go / 60) % 60;
							int togo_time_h = time_to_go / 3600;
							int time_s = (int) time % 60;
							int time_min = (int) (time / 60) % 60;
							int time_h = time / 3600;
							double iterations_per_sec = N_counter / time;
							//double avgDEerror = DEerror / DE_counterForDEerror*100.0;
							double avgMissedDE = (double) Missed_DE_counter / Pixel_counter * 100.0;
							sprintf(progressText, "%.3f%%, to go %dh%dm%ds, elapsed %dh%dm%ds, iters/s %.0f, Missed DE %.3f%%", percent_done, togo_time_h, togo_time_min, togo_time_s, time_h,
									time_min, time_s, iterations_per_sec, avgMissedDE);
							gtk_progress_bar_set_text(GTK_PROGRESS_BAR(Interface.progressBar), progressText);
							gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(Interface.progressBar), percent_done / 100.0);
						}
					}
					//refreshing GUI
					if (image->IsPreview() || netRender->IsClient())
					{
						while (gtk_events_pending())
							gtk_main_iteration();
					}

					done2 = 0;
					for(int i=0; i<height; i++)
					{
						if(thread_done[i] > 0)
						{
							done2++;
						}
					}
				}
			}
			//when rendered all of image lines wait for finishing threads
			for (int i = 0; i < NR_THREADS; i++)
			{
				g_thread_join(Thread[i]);
				WriteLogDouble("Thread finish confirmed", i);
				//printf("Rendering thread #%d finished\n", i + 1);
			}

			//terminate rendering by all clients
			if (netRender->IsServer())
			{
				for (int i = 0; i < netRender->getNoOfClients(); i++)
				{
					netRender->sendDataToClient(NULL, 0, (char*)"stp", i, param.fractal.frameNo);
				}
			}

			if (programClosed) break;
		} //next progress
		printf("\n");

		//refreshing image

		//*** postprocessing

		image->CompileImage();
		WriteLog("Image compiled");
		if (param.SSAOEnabled && !programClosed && !netRender->IsClient())
		{
			PostRendering_SSAO(image, param.doubles.persp, param.SSAOQuality, param.perspectiveType, param.quiet);
			WriteLog("SSAO rendered");
		}
		if (param.DOFEnabled && !programClosed && !netRender->IsClient())
		{
			double DOF_focus = pow(10, param.doubles.DOFFocus / 10.0 - 16.0);
			PostRendering_DOF(image, param.doubles.DOFRadius * width / 1000.0, DOF_focus);
			WriteLog("DOF rendered");
		}

		if (!noGUI)
		{
			if (image->IsPreview())
			{
				image->ConvertTo8bit();
				WriteLog("Image converted to 8-bit");
				image->UpdatePreview();
				WriteLog("Preview created");
				if (outputDarea != NULL)
				{
					DrawHistogram();
					DrawHistogram2();
					WriteLog("Histograms refreshed");
					image->RedrawInWidget(outputDarea);
					WriteLog("Image redrawn in window");
				}
			}
		}

		if (!noGUI)
		{
			if (outputDarea != NULL)
			{
				char progressText[1000];
				double time = real_clock() - start_time;
				int time_s = (int) time % 60;
				int time_min = (int) (time / 60) % 60;
				int time_h = time / 3600;
				double iterations_per_sec = N_counter / time;
				double avg_N = (double) N_counter / Loop_counter;
				double avg_DE = (double) DE_counter / Pixel_counter;
				double avgDEerror = DEerror / DE_counterForDEerror * 100.0;
				double avgMissedDE = (double) Missed_DE_counter / Pixel_counter * 100.0;
				sprintf(progressText, "Render time %dh%dm%ds, iters/s %.0f, avg. N %.1f, avg. DEsteps %.1f, DEerror %.1f%%, MissedDE %.3f%%", time_h, time_min, time_s, iterations_per_sec,
						avg_N, avg_DE, avgDEerror, avgMissedDE);
				gtk_progress_bar_set_text(GTK_PROGRESS_BAR(Interface.progressBar), progressText);
				gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(Interface.progressBar), 1.0);
			}
			if (image->IsPreview())
			{
				while (gtk_events_pending())
					gtk_main_iteration();
			}
		}
		delete[] thread_done;
		delete[] thread_param;
		delete[] Thread;
		delete[] err;
		delete[] linesToSend;
		if (lineOfImage) delete[] lineOfImage;
	}
	else
	{
#ifdef CLSUPPORT
		printf("OpenCL rendering\n");
		clSupport->SetSize(image->GetWidth(), image->GetHeight());

		sClInBuff *inCLBuff = clSupport->GetInBuffer1();
		sClInConstants *inCLConstants = clSupport->GetInConstantBuffer1();
		Params2Cl(&param, inCLBuff, inCLConstants);

		//calculating vectors for AmbientOcclusion

		int counter = 0;
		int lightMapWidth = param.lightmapTexture->Width();
		int lightMapHeight = param.lightmapTexture->Height();
		for (double b = -0.49 * M_PI; b < 0.49 * M_PI; b += 1.0 / param.globalIlumQuality)
		{
			for (double a = 0.0; a < 2.0 * M_PI; a += ((2.0 / param.globalIlumQuality) / cos(b)))
			{
				CVector3 d;
				d.x = cos(a + b) * cos(b);
				d.y = sin(a + b) * cos(b);
				d.z = sin(b);
				inCLBuff->vectorsAround[counter].x = d.x;
				inCLBuff->vectorsAround[counter].y = d.y;
				inCLBuff->vectorsAround[counter].z = d.z;
				int X = (int) ((a + b) / (2.0 * M_PI) * lightMapWidth + lightMapWidth * 8.5) % lightMapWidth;
				int Y = (int) (b / (M_PI) * lightMapHeight + lightMapHeight * 8.5) % lightMapHeight;
				inCLBuff->vectorsAroundColours[counter].x = param.lightmapTexture->FastPixel(X, Y).R / 256.0;
				inCLBuff->vectorsAroundColours[counter].y = param.lightmapTexture->FastPixel(X, Y).G / 256.0;
				inCLBuff->vectorsAroundColours[counter].z = param.lightmapTexture->FastPixel(X, Y).B / 256.0;
				if (inCLBuff->vectorsAroundColours[counter].x > 0.05 || inCLBuff->vectorsAroundColours[counter].y > 0.05 || inCLBuff->vectorsAroundColours[counter].z > 0.05)
				{
					counter++;
				}
				if (counter >= 10000) break;
			}
			if (counter >= 10000) break;
		}
		if (counter == 0)
		{
			counter = 1;
			inCLBuff->vectorsAround[0].x = 0;
			inCLBuff->vectorsAround[0].y = 0;
			inCLBuff->vectorsAround[0].z = 0;

			inCLBuff->vectorsAroundColours[0].x = 0;
			inCLBuff->vectorsAroundColours[0].y = 0;
			inCLBuff->vectorsAroundColours[0].z = 0;
		}
		inCLConstants->params.AmbientOcclusionNoOfVectors = counter;
		printf("Ambient occlusion counter %d\n", counter);

		clSupport->SetParams(inCLBuff, inCLConstants, param.fractal.formula);
		start_time = real_clock();
		clSupport->Render(image, outputDarea);
		double time = real_clock() - start_time;
		if (image->IsPreview())
		{
			image->ConvertTo8bit();
			image->UpdatePreview();
			image->RedrawInWidget(outputDarea);
			while (gtk_events_pending())
				gtk_main_iteration();
		}
		if (outputDarea != NULL)
		{
			char progressText[1000];
			sprintf(progressText, "OpenCL rendering time: %f seconds", time);
			gtk_progress_bar_set_text(GTK_PROGRESS_BAR(Interface.progressBar), progressText);
			gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(Interface.progressBar), 1.0);
		}
#endif
	}
}

//******************************** Get number of CPU cores *************

int get_cpu_count()
{
	int ret;

#ifdef WIN32 /* WINDOWS */
	SYSTEM_INFO info;

	GetSystemInfo(&info);
	ret = info.dwNumberOfProcessors;
#elif defined(__sgi)
	ret = (int) sysconf(_SC_NPROC_ONLN);
#else               /*other unix - try sysconf*/
	ret = (int) sysconf(_SC_NPROCESSORS_ONLN);
#endif  /* WINDOWS */
	return ret;
}

//********************************** MAIN *******************************
int main(int argc, char *argv[])
{
	//read $home env variable
	const char *homedir;

	setlocale(LC_ALL, "");

#ifdef WIN32 /* WINDOWS */
	homedir = getenv("USERPROFILE");
#else               /*other unix - try sysconf*/
	homedir = getenv("HOME");
#endif  /* WINDOWS */

#ifdef WIN32 /* WINDOWS */
  sharedDir = new char[MAXPATHLEN];
  char pathCWD[MAXPATHLEN];
  getcwd(pathCWD, MAXPATHLEN);
  strcpy(sharedDir, (string(pathCWD)+"/").c_str());
#else               /*other unix - try sysconf*/
	sharedDir = new char[1000];
	strcpy(sharedDir, (string(SHARED_DIR)+"/").c_str());
#endif  /* WINDOWS */

	//logfile
#ifdef WIN32 /* WINDOWS */
	logfileName="log.txt";
#else
	logfileName=string(homedir)+ "/.mandelbulber_log.txt";
#endif
	FILE *logfile = fopen(logfileName.c_str(), "w");
	fclose(logfile);

	printf("Log file: %s\n", logfileName.c_str());
	WriteLog("Log file created");

	//initialising GTK+
	bool result = ReadComandlineParams(argc, argv);
	WriteLogDouble("Parameters read from console", argc);
	if (result == false)
	{
		printf("Failed to start program. Wrong command syntax\n");
		return -1;
	}

	if (noGUI)
	{
		printf("noGUI mode\n");
		WriteLog("noGUI mode");
	}
	else
	{
		gtk_init(&argc, &argv);
		WriteLog("GTK initialised");
	}

	//detecting number of CPU cores
	NR_THREADS = get_cpu_count();

#ifdef ONETHREAD //for debbuging
	NR_THREADS = 1;
#endif

	//NR_THREADS = 1;

	printf("Detected %d CPUs\n", NR_THREADS);
	WriteLogDouble("CPUs detected", NR_THREADS);

	//lockout for refreshing image during program startup
	Interface_data.disableInitRefresh = true;

	//allocating memory for image in window
	mainImage.CreatePreview(1.0);
	WriteLog("Memory allocated for preview");

	//allocating memory for lights
	Lights = new sLight[10000];
	WriteLog("memory for lights allocated");
	printf("Memory allocated\n");

	timeline.reset(new cTimeline);

	//initialising g_thread
	if (!g_thread_supported())
	{
		g_thread_init(NULL);
		gdk_threads_init(); // Called to initialize internal mutex "gdk_threads_mutex".
		printf("g_thread supported\n");
	}
	else
	{
		printf("g_thread already initialised or not supported\n");
		//return 1;
	}
	WriteLog("g_thread initialised");

	//OpenCL

	clSupport = new CclSupport();
	//clSupport->Init();

	sParamRender fractParamDefault;
	memset(&fractParamDefault, 0, sizeof(sParamRender));
	WriteLog("allocated memory for default parameters");

	//data directory location
#ifdef WIN32 /* WINDOWS */
	sprintf(data_directory, "%s/mandelbulber", homedir);
#else
	sprintf(data_directory, "%s/.mandelbulber", homedir);
#endif
	printf("Default data directory: %s\n", data_directory);

	//create data directory if not exists
	DIR *dir;
	dir = opendir(data_directory);
	if (dir != NULL) (void) closedir(dir);
#ifdef WIN32
	else mkdir(data_directory);
#else
	else mkdir(data_directory, (S_IRUSR | S_IWUSR | S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH));
#endif
	WriteLog("Data directory");

	//create settings directory if not exists
	string settingsDir = string(data_directory) + "/settings";
	dir = opendir(settingsDir.c_str());
	if (dir != NULL) (void) closedir(dir);
#ifdef WIN32
	else mkdir(settingsDir.c_str());
#else
	else mkdir(settingsDir.c_str(), (S_IRUSR | S_IWUSR | S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH));
#endif
	WriteLog("Settings directory");

	//create settings directory if not exists
	string imagesDir = string(data_directory) + "/images";
	dir = opendir(imagesDir.c_str());
	if (dir != NULL) (void) closedir(dir);
#ifdef WIN32
	else mkdir(imagesDir.c_str());
#else
	else mkdir(imagesDir.c_str(), (S_IRUSR | S_IWUSR | S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH));
#endif
	WriteLog("Images directory");

	//create keyframes directory if not exists
	string keyframesDir = string(data_directory) + "/keyframes";
	dir = opendir(keyframesDir.c_str());
	if (dir != NULL) (void) closedir(dir);
#ifdef WIN32
	else mkdir(keyframesDir.c_str());
#else
	else mkdir(keyframesDir.c_str(), (S_IRUSR | S_IWUSR | S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH));
#endif
	WriteLog("Keyframes directory");

	//create paths directory if not exists
	string pathsDir = string(data_directory) + "/paths";
	dir = opendir(pathsDir.c_str());
	if (dir != NULL) (void) closedir(dir);
#ifdef WIN32
	else mkdir(pathsDir.c_str());
#else
	else mkdir(pathsDir.c_str(), (S_IRUSR | S_IWUSR | S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH));
#endif
	WriteLog("Paths directory");

	//create undo directory if not exists
	string undoDir = string(data_directory) + "/undo";
	dir = opendir(undoDir.c_str());
	if (dir != NULL) (void) closedir(dir);
#ifdef WIN32
	else mkdir(undoDir.c_str());
#else
	else mkdir(undoDir.c_str(), (S_IRUSR | S_IWUSR | S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH));
#endif
	WriteLog("Undo directory");

	string defaults = string(data_directory) + "/.defaults";
	if(!FileIfExists(defaults.c_str()))
	{
		fcopy((string(sharedDir)+"/defaults").c_str(),defaults.c_str());
		WriteLog("Defaults copied");
	}

	strcpy(lastFilenameSettings, "settings/default.fract");
	strcpy(lastFilenameImage, "images/image.jpg");
	strcpy(lastFilenamePalette, (string(sharedDir)+"textures/colour palette.jpg").c_str());

	bool noGUIsettingsLoaded = false;
	if (noGUI)
	{
		if (LoadSettings(noGUIdata.settingsFile, noGUIdata.fractparams))
		{
			noGUIsettingsLoaded = true;
			WriteLog("Parameters loaded in noGUI mode");
		}
	}

	//initialise netRender
	netRender = new CNetRender(MANDELBULBER_VERSION * 1000, NR_THREADS);

	//set program home directory
	if (!chdir(data_directory))
	{
		//update of reference file with defaults
		SaveSettings(".defaults", fractParamDefault, false);
		WriteLog("Defaults regenerated");

		//loading undo buffer status
		undoBuffer.LoadStatus();
		WriteLog("Undo buffer status loaded");

		//reading default configuration in GUI mode
		if (!noGUI)
		{
			if (LoadSettings((string(sharedDir)+"examples/default.fract").c_str(), fractParamDefault))
			{
				WriteLog("Default settings loaded");
				printf("Default settings loaded successfully (%s)\n",(string(sharedDir)+"examples/default.fract").c_str());
				//creating GTK+ GUI
				Params2InterfaceData(&fractParamDefault);
				CreateInterface(&fractParamDefault);
			}
			else
			{
				printf("Can't open default settings file: %s\n",(string(sharedDir)+"examples/default.fract").c_str());
				WriteLog("Can't open default settings file");
				WriteLog(data_directory);

				GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window_interface), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CANCEL,
						"Error! Can't open default settings file\n%s",(string(sharedDir)+"examples/default.fract").c_str());
				gtk_dialog_run(GTK_DIALOG(dialog));
				gtk_widget_destroy(dialog);
			}
		}
		else
		{
		  g_type_init();
			if(!noGUIdata.netrenderMode)
			{
				if (!noGUIsettingsLoaded)
				{
					char settingsFile2[1000];
					sprintf(settingsFile2, "settings/%s", noGUIdata.settingsFile);
					if (!LoadSettings(settingsFile2, noGUIdata.fractparams))
					{
						printf("ERROR! Can't open settings file: %s\n", noGUIdata.settingsFile);
						return -1;
					}
				}
				WriteLog("Default settings loaded");
				Interface_data.animMode = noGUIdata.animMode;
				Interface_data.playMode = noGUIdata.playMode;
				Interface_data.recordMode = false;
				Interface_data.continueRecord = false;
				Interface_data.keyframeMode = noGUIdata.keyframeMode;
				MainRender();
				WriteLog("Rendering completely finished");
			}
			else
			{
				printf("Starting net client application\n");
				NoGUIClientEnable();
			}
		}

		WriteLog("Memory for parameters released");
	}
	else
	{
		printf("Can't access default data directory: %s\n", data_directory);
		WriteLog("Can't access default data directory");
		WriteLog(data_directory);
	}
	return 0;
}

//Init parameters
void InitMainParameters(sParamRender *fractParam)
{
	WriteLog("Memory allocated for fractal parameters");

	//reading parameters from interface
	if (!noGUI)
	{
		ReadInterface(fractParam);
		printf("Data has been read from interface\n");
		WriteLog("Data read from interface");
	}
	else
	{
		memcpy(fractParam, &noGUIdata.fractparams, sizeof(sParamRender));
		Params2InterfaceData(&noGUIdata.fractparams);
		ReadInterface(fractParam);
		Interface_data.imageFormat = noGUIdata.imageFormat;
		WriteLog("Data read from interface");
	}

	//animation/render mode
	fractParam->playMode = Interface_data.playMode;
	fractParam->animMode = Interface_data.animMode;
	fractParam->recordMode = Interface_data.recordMode;
	fractParam->continueRecord = Interface_data.continueRecord;
}

//Init main image and window
void InitMainImage(cImage *image, int width, int height, double previewScale, GtkWidget *drawingArea)
{
	image->ChangeSize(width, height);
	WriteLog("complexImage allocated");
	printf("Memory for image: %d MB\n", image->GetUsedMB());

	if(!noGUI)
		image->CreatePreview(previewScale);

	if (!noGUI)
	{
		gtk_widget_set_size_request(drawingArea, image->GetPreviewWidth(), image->GetPreviewHeight());
	}

	WriteLog("rgbbuf allocated");

	//waiting for refresh window
	//g_usleep(100000);
	if (!noGUI)
	{
		for (int i = 0; i < 10; i++)
		{
			while (gtk_events_pending())
				gtk_main_iteration();
		}
	}
	WriteLog("Windows refreshed");
	printf("Memory for image reallocated\n");
}

bool LoadTextures(sParamRender *params)
{
	//loading texture for environment mapping
	if(params->doubles.imageAdjustments.reflect > 0 && !params->imageSwitches.raytracedReflections)
	{
		params->envmapTexture = new cTexture(params->file_envmap);
		if (params->envmapTexture->IsLoaded())
		{
			printf("Environment map texture loaded\n");
			WriteLog("Environment map texture loaded");
		}
		else
		{
			printf("Error! Can't open envmap texture: %s\n", params->file_envmap);
			WriteLog("Error! Can't open envmap texture");
			WriteLog(params->file_envmap);
			if (!noGUI)
			{
				GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window_interface), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CANCEL, "Error loading envmap texture file: %s",
						params->file_envmap);
				gtk_dialog_run(GTK_DIALOG(dialog));
				gtk_widget_destroy(dialog);
			}
			//isRendering = false;
			//return false;
		}
	}
	else
	{
		params->envmapTexture = new cTexture;
	}

	//loading texture for ambient occlusion light map
	if(params->global_ilumination && !params->fastGlobalIllumination)
	{
		params->lightmapTexture = new cTexture(params->file_lightmap);
		if (params->lightmapTexture->IsLoaded())
		{
			printf("Ambient occlusion light map texture loaded\n");
			WriteLog("Ambient occlusion light map texture loaded");
		}
		else
		{
			printf("Error! Can't open ambient occlusion light map texture:%s\n", params->file_lightmap);
			WriteLog("Error! Can't open ambient occlusion light map texture");
			WriteLog(params->file_lightmap);
			if (!noGUI)
			{
				GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window_interface), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CANCEL,
						"Error! Can't open ambient occlusion light map texture:\n%s", params->file_lightmap);
				gtk_dialog_run(GTK_DIALOG(dialog));
				gtk_widget_destroy(dialog);
			}
			//isRendering = false;
			//return false;
		}
	}
	else
	{
		params->lightmapTexture = new cTexture;
	}

	//reading background texture
	if(params->textured_background)
	{
		params->backgroundTexture = new cTexture(params->file_background);
		if (params->backgroundTexture->IsLoaded())
		{
			printf("Background texture loaded\n");
			WriteLog("Background texture loaded");
		}
		else
		{
			printf("Error! Can't open background texture:%s\n", params->file_background);
			WriteLog("Error! Can't open background texture");
			WriteLog(params->file_background);
			if (!noGUI)
			{
				GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window_interface), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CANCEL,
						"Error! Can't open background texture:\n%s", params->file_background);
				gtk_dialog_run(GTK_DIALOG(dialog));
				gtk_widget_destroy(dialog);
			}
			//isRendering = false;
			//return false;
		}
	}
	else
	{
		params->backgroundTexture = new cTexture;
	}
	return true;
}

//****************************8 MAIN called by "Render" button
void MainRender(void)
{
	isRendering = true;
	programClosed = false;

	//allocating memory for fractal parameters
	sParamRender fractParam;

	if (noGUI)
	{
		memcpy(Interface_data.palette, noGUIdata.fractparams.palette, sizeof(sRGB)*256);
	}

	InitMainParameters(&fractParam);

	if (!LoadTextures(&fractParam)) return;

	//image size
	int width = fractParam.image_width;
	int height = fractParam.image_height;
	InitMainImage(&mainImage, width, height, Interface_data.imageScale, renderWindow.drawingArea);

	if (!noGUI)
	{
		DrawPalette(fractParam.palette);
		WriteLog("Palette refreshed on GUI");
	}
	//placing random Lights
	if (lightsPlaced == 0)
	{
		PlaceRandomLights(&fractParam, false);
		WriteLog("Lights placed");
	}
	printf("Lights placed\n");

	FILE *pFile_coordinates = NULL;
	//erasing file with animation path in path record mode
	if (fractParam.animMode && !Interface_data.keyframeMode)
	{
		if (fractParam.recordMode & !fractParam.continueRecord)
		{
			pFile_coordinates = fopen(fractParam.file_path, "w");
			fclose(pFile_coordinates);
			WriteLog("File for coordinates prepared");
		}
	}

	//auxiliary arrays
	string filename2;
	double distance = 0;
	char label_text[1000];

	//Opening file with flight path for animation mode
	if ((fractParam.playMode || fractParam.continueRecord) && !Interface_data.keyframeMode)
	{
		pFile_coordinates = fopen(fractParam.file_path, "r");
		WriteLog("File with coordinates opened");
	}

	//checking number of keyframes in keyframe animation mode
	int maxKeyNumber = 0;
	if (Interface_data.keyframeMode)
	{
		do
		{
			filename2=IndexFilename(fractParam.file_keyframes, "fract", maxKeyNumber);
			maxKeyNumber++;
		} while (FileIfExists(filename2.c_str()));
		WriteLog("Keyframes counted");
	}
	else
	{
		maxKeyNumber = 1;
	}
	maxKeyNumber--;
	if (Interface_data.keyframeMode) printf("Found %d keyframes\n", maxKeyNumber);

	//loading keyframes in keyframe animation mode

	CMorph morphParamRender(maxKeyNumber, sizeof(sParamRenderD) / sizeof(double));
	CMorph morphIFS(maxKeyNumber, sizeof(sFractalIFSD) / sizeof(double));
	CMorph morphMandelbox(maxKeyNumber, sizeof(sFractalMandelboxD) / sizeof(double));
	CMorph morphFractal(maxKeyNumber, sizeof(sFractalD) / sizeof(double));

	morphParamRender.SetFramesPerKey(fractParam.framesPerKeyframe);
	morphIFS.SetFramesPerKey(fractParam.framesPerKeyframe);
	morphMandelbox.SetFramesPerKey(fractParam.framesPerKeyframe);
	morphFractal.SetFramesPerKey(fractParam.framesPerKeyframe);

	if (Interface_data.keyframeMode)
	{
		for (int keyNumber = 0; keyNumber < maxKeyNumber; keyNumber++)
		{
			filename2=IndexFilename(fractParam.file_keyframes, "fract", keyNumber);

			sParamRender fractParamLoaded;
			LoadSettings(filename2.c_str(), fractParamLoaded, true);
			WriteLogDouble("Keyframe loaded", keyNumber);

			morphParamRender.AddData(keyNumber, (double*) &fractParamLoaded.doubles);
			morphIFS.AddData(keyNumber, (double*) &fractParamLoaded.fractal.IFS.doubles);
			morphMandelbox.AddData(keyNumber, (double*) &fractParamLoaded.fractal.mandelbox.doubles);
			morphFractal.AddData(keyNumber, (double*) &fractParamLoaded.fractal.doubles);

			WriteLogDouble("Keyframe data added to data structures", keyNumber);
		}
		printf("Keyframes loaded\n");
	}

	bool autoSaveImage = false;
	if (noGUI)
	{
		if(netRender->IsClient())
		{
			autoSaveImage = false;
		}
		else
		{
			autoSaveImage = true;
		}
	}
	else
	{
		autoSaveImage = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkAutoSaveImage));
	}

	bool foundLastFrame = false;

	if (!fractParam.continueRecord) foundLastFrame = true;

	//głowna pętla renderowania kolejnych klatek animacji
	int startFrame = 0;
	int endFrame = 100000;

	if (Interface_data.keyframeMode || Interface_data.playMode)
	{
		startFrame = fractParam.startFrame;
		endFrame = fractParam.endFrame;

		if (noGUI && noGUIdata.startFrame > 0) startFrame = noGUIdata.startFrame;
		if (noGUI && noGUIdata.endFrame < 99999) endFrame = noGUIdata.endFrame;
		if(endFrame > maxKeyNumber * fractParam.framesPerKeyframe && Interface_data.keyframeMode) endFrame = maxKeyNumber * fractParam.framesPerKeyframe;
	}

	//rewinding coordinates file to the first startFame
	if (fractParam.playMode)
	{
		for (int i = 0; i < startFrame; i++)
		{
			int n;
			n = fscanf(pFile_coordinates, "%lf %lf %lf %lf %lf", &fractParam.doubles.vp.x, &fractParam.doubles.vp.y, &fractParam.doubles.vp.z, &fractParam.doubles.alpha,
					&fractParam.doubles.beta);
			if (n <= 0)
			{
				fclose(pFile_coordinates);
				return;
			}
		}
	}

	cImage *secondEyeImage = 0;
	unsigned char *stereoImage = 0;
	if (fractParam.stereoEnabled)
	{
		autoSaveImage = true;
		secondEyeImage = new cImage(width, height);
		secondEyeImage->CreatePreview(Interface_data.imageScale);
		secondEyeImage->SetImageParameters(fractParam.doubles.imageAdjustments);
		stereoImage = new unsigned char[width * height * 3 * 2];
	}

	CVector3 last_vp_position = fractParam.doubles.vp;

	int tiles = fractParam.noOfTiles;

	int lastDeltaXm, lastDeltaYm;
	double spaceShipAngle = 0.0;
	double last_spaceShipAngle = 0.0;

	start_time = real_clock();

	printf("************ Rendering frames *************\n");
	WriteLog("************ Rendering frames *************");
	for (int index = startFrame; index < endFrame; index++)
	{
		WriteLogDouble("Started rendering frame", index);
		//printf("file index %d\n", index);

		//animation with recorded flight path
		double speed = 0.02;

		if(!netRender->IsClient())
		{
			fractParam.fractal.frameNo = index;
		}
		if (fractParam.animMode && !Interface_data.keyframeMode)
		{
			sprintf(label_text, "Frame: %d", index);
			if (!noGUI)
			{
				gtk_label_set_text(GTK_LABEL(Interface.label_animationFrame), label_text);
			}

			//calculating of mouse pointer position
			int deltaXm, deltaYm;

			deltaXm = x_mouse - width * mainImage.GetPreviewScale() / 2;
			deltaYm = y_mouse - height * mainImage.GetPreviewScale() / 2;
			int XmStep = deltaXm - lastDeltaXm;
			int YmStep = deltaYm - lastDeltaYm;

			if (fractParam.recordMode)
			{
				/*
				fractParam.doubles.alpha -= 0.0003 * delta_xm;
				fractParam.doubles.beta += 0.0003 * delta_ym;
				*/
				double rotation_step = 0.0001;

				spaceShipAngle -= deltaXm * rotation_step;
				fractParam.doubles.gamma -= deltaXm * rotation_step;

				CRotationMatrix mRot;
				mRot.RotateZ(fractParam.doubles.alpha);
				mRot.RotateX(fractParam.doubles.beta);
				mRot.RotateY(fractParam.doubles.gamma);
				mRot.RotateX(-deltaYm * rotation_step);
				mRot.RotateZ(deltaXm * rotation_step);

				fractParam.doubles.alpha = -mRot.GetAlfa();
				fractParam.doubles.beta = -mRot.GetBeta();
				fractParam.doubles.gamma = -mRot.GetGamma();

			}

			//calculation of distance to fractal surface
			distance = CalculateDistance(fractParam.doubles.vp, fractParam.fractal);
			WriteLogDouble("Distance calculated", distance);

			sprintf(label_text, "Estimated distance to fractal surface: %g", distance);
			if (!noGUI) gtk_label_set_text(GTK_LABEL(Interface.label_animationDistance), label_text);

			sprintf(label_text, "Flight speed: %g", distance * speed);
			if (!noGUI) gtk_label_set_text(GTK_LABEL(Interface.label_animationSpeed), label_text);

			if (distance > 5.0) distance = 5.0;

			//auto-calculate of zoom value
			fractParam.doubles.zoom = distance / 1000;

			//path record mode
			if (fractParam.recordMode && foundLastFrame)
			{

				speed = atof(gtk_entry_get_text(GTK_ENTRY(Interface.edit_animationDESpeed)));

				//calculation of step direction vector
				double delta_x = -sin(fractParam.doubles.alpha) * cos(fractParam.doubles.beta) * distance * speed;
				double delta_y = cos(fractParam.doubles.alpha) * cos(fractParam.doubles.beta) * distance * speed;
				double delta_z = sin(fractParam.doubles.beta) * distance * speed;

				//step
				fractParam.doubles.vp.x += delta_x;
				fractParam.doubles.vp.y += delta_y;
				fractParam.doubles.vp.z += delta_z;

				//saving coordinates to file
				pFile_coordinates = fopen(fractParam.file_path, "a");
				fprintf(pFile_coordinates, "%.11f %.11f %.11f %f %f %f\n", fractParam.doubles.vp.x, fractParam.doubles.vp.y, fractParam.doubles.vp.z, fractParam.doubles.alpha,
						fractParam.doubles.beta, fractParam.doubles.gamma);
				fclose(pFile_coordinates);
				WriteLog("Coordinates saved");
			}

			//path play mode
			if (fractParam.playMode || (fractParam.continueRecord && !foundLastFrame))
			{
				//loading coordinates
				int n;
				n = fscanf(pFile_coordinates, "%lf %lf %lf %lf %lf %lf", &fractParam.doubles.vp.x, &fractParam.doubles.vp.y, &fractParam.doubles.vp.z, &fractParam.doubles.alpha,
						&fractParam.doubles.beta, &fractParam.doubles.gamma);
				if (n <= 0)
				{
					foundLastFrame = true;
					fclose(pFile_coordinates);
				}
				WriteLog("Coordinates loaded");
			}
		}

		//Catmull-Rom interpolation in keyframe animation mode
		if (fractParam.animMode && Interface_data.keyframeMode)
		{
			morphParamRender.CatmullRom(index, (double*) &fractParam.doubles);
			morphIFS.CatmullRom(index, (double*) &fractParam.fractal.IFS.doubles);
			morphMandelbox.CatmullRom(index, (double*) &fractParam.fractal.mandelbox.doubles);
			morphFractal.CatmullRom(index, (double*) &fractParam.fractal.doubles);

			WriteLog("Splines calculated");

			if (fractParam.doubles.zoom < 1e-15) fractParam.doubles.zoom = 1e-15;
			fractParam.doubles.resolution = 1.0 / fractParam.image_width;
			sImageAdjustments imageAdjustments = fractParam.doubles.imageAdjustments;
			mainImage.SetImageParameters(imageAdjustments);

			sprintf(label_text, "Frame: %d, Keyframe %f", index, (double) index / fractParam.framesPerKeyframe);
			if (!noGUI) gtk_label_set_text(GTK_LABEL(Interface.label_keyframeInfo), label_text);
			WriteLog("Calculated additional data for splines");
		}

		RecalculateIFSParams(fractParam.fractal);
		WriteLog("IFS params recalculated");

		CreateFormulaSequence(fractParam.fractal);
		WriteLog("Formula sequence created");

		PlaceRandomLights(&fractParam, true);

		if (!noGUI)
		{
			if (fractParam.animMode) WriteInterface(&fractParam);
			else WriteInterface(&fractParam);
			WriteLog("GUI data refreshed");
		}

		bool tilesDone = false;
		for (int tile = 0; tile < tiles*tiles; tile++)
		{
			int index2 = index * tiles * tiles + tile;
			fractParam.tileCount = tile;
			if(tiles > 1)
			{
				printf("---------- Tile: %d -------------\n", index2);
			}

			if (Interface_data.imageFormat == imgFormatJPG && tiles == 1)
			{
				filename2 = IndexFilename(fractParam.file_destination, "jpg", index2);
			}
			else if(tiles == 1)
			{
				filename2 = IndexFilename(fractParam.file_destination, "png", index2);
			}
			else
			{
				filename2 = IndexFilename(fractParam.file_destination, "tile", index2);
			}
			FILE *testFile;
			testFile = fopen(filename2.c_str(), "r");
			if ((testFile != NULL && !fractParam.recordMode && (autoSaveImage || fractParam.animMode || tiles > 1)) || (!foundLastFrame && fractParam.continueRecord))
			{
				if (!fractParam.animMode)
				{
					printf("Output file (%s) exists. Skip to next file index\n", filename2.c_str());
				}
			}
			else
			{

				//renderowanie fraktala
				//if ((index % frame_step == 0) || record_mode)

				if (fractParam.animMode)
				{
					distance = CalculateDistance(fractParam.doubles.vp, fractParam.fractal);
					printf("---------- index: %d -------------\n", index);
					printf("Distance = %e\n", distance);
					printf("alpha = %f, beta = %f\n", fractParam.doubles.alpha, fractParam.doubles.beta);
					printf("x = %.8f, y = %.8f, z = %.8f\n", fractParam.doubles.vp.x, fractParam.doubles.vp.y, fractParam.doubles.vp.z);
					if (!noGUI)
					{
						char distanceString[1000];
						CVector3 delta_vp = fractParam.doubles.vp - last_vp_position;
						double actual_speed = delta_vp.Length();
						sprintf(distanceString, "Estimated viewpoint distance to the surface: %g, actual speed: %g", distance, actual_speed);
						gtk_label_set_text(GTK_LABEL(Interface.label_NavigatorEstimatedDistance), distanceString);
					}
				}

				CVector3 eyeLeft, eyeRight;
				int numberOfEyes = 1;
				if (fractParam.stereoEnabled)
				{
					CRotationMatrix mRotForEyes;
					mRotForEyes.RotateZ(fractParam.doubles.alpha);
					mRotForEyes.RotateX(fractParam.doubles.beta);
					mRotForEyes.RotateY(fractParam.doubles.gamma);
					CVector3 baseVectorForEyes(0.5 * fractParam.doubles.stereoEyeDistance, 0, 0);
					eyeLeft = fractParam.doubles.vp - mRotForEyes.RotateVector(baseVectorForEyes);
					eyeRight = fractParam.doubles.vp + mRotForEyes.RotateVector(baseVectorForEyes);
					numberOfEyes = 2;

				}

				//if number of eyes = 2 (stereo) then render two times
				for (int eye = 0; eye < numberOfEyes; eye++)
				{
					if (fractParam.stereoEnabled)
					{
						if (eye == 0)
						{
							fractParam.doubles.vp = eyeLeft;
						}
						else if (eye == 1)
						{
							fractParam.doubles.vp = eyeRight;
						}
					}

					Interface_data.disableInitRefresh = false;

					if (eye == 0)
					{
						mainImage.ClearImage();
						WriteLog("Image cleared");
						Render(fractParam, &mainImage, renderWindow.drawingArea);
						WriteLog("Image rendered");
					}
					else if (eye == 1)
					{
						secondEyeImage->ClearImage();
						WriteLog("Image cleared");
						Render(fractParam, secondEyeImage, renderWindow.drawingArea);
						WriteLog("Image rendered");
						MakeStereoImage(&mainImage, secondEyeImage, stereoImage);
						WriteLog("Stereo image made");
						if (!noGUI) StereoPreview(&mainImage, stereoImage);
						filename2 = IndexFilename(fractParam.file_destination, "jpg", index);
						SaveJPEG(filename2.c_str(), 100, width * 2, height, (JSAMPLE*) stereoImage);
						WriteLog("Stereo image saved");
						if (!noGUI)
						{
							char progressText[1000];
							sprintf(progressText, "Stereoscopic image was saved to: %s", filename2.c_str());
							gtk_progress_bar_set_text(GTK_PROGRESS_BAR(Interface.progressBar), progressText);
							StereoPreview(&mainImage, stereoImage);
						}
					}

					//save image
					if ((autoSaveImage || fractParam.animMode || tiles > 1) && !fractParam.stereoEnabled && !programClosed)
					{
						if(tiles>1)
						{
							FILE * binFile;
							filename2 = IndexFilename(fractParam.file_destination, "tile", index2);
							binFile = fopen(filename2.c_str(), "wb");
							fwrite(mainImage.GetImage16Ptr(), 1, sizeof(sRGB16)*width*height, binFile);
							fclose(binFile);
						}
						else
						{
							unsigned char *rgbbuf2 = mainImage.ConvertTo8bit();
							if (Interface_data.imageFormat == imgFormatJPG)
							{
								filename2 = IndexFilename(fractParam.file_destination, "jpg", index2);
								SaveJPEG(filename2.c_str(), 100, width, height, (JSAMPLE*) rgbbuf2);
							}
							else if (Interface_data.imageFormat == imgFormatPNG)
							{
								filename2 = IndexFilename(fractParam.file_destination, "png", index2);
								SavePNG(filename2.c_str(), 100, width, height, (png_byte*) rgbbuf2);
							}
							else if (Interface_data.imageFormat == imgFormatPNG16)
							{
								filename2 = IndexFilename(fractParam.file_destination, "png", index2);
								SavePNG16(filename2.c_str(), 100, width, height, mainImage.GetImage16Ptr());
							}
							else if (Interface_data.imageFormat == imgFormatPNG16Alpha)
							{
								filename2 = IndexFilename(fractParam.file_destination, "png", index2);
								SavePNG16Alpha(filename2.c_str(), 100, width, height, &mainImage);
							}
						}
						printf("Image saved: %s\n", filename2.c_str());
						WriteLog("Image saved");
						WriteLog(filename2.c_str());
					}
				}
				//terminate animation loop when not animation mode
				if (!fractParam.animMode && tile >= tiles*tiles-1)
				{
					index = 9999999;
				}
			}

			last_vp_position = fractParam.doubles.vp;

			if (testFile != NULL)
			{
				fclose(testFile);
			}

			if (programClosed) break;

			if(tile == tiles * tiles -1) tilesDone = true;
		}
		if(tiles > 1 && tilesDone)
		{
			SaveFromTilesPNG16(fractParam.file_destination, width, height, tiles);
		}
		if (programClosed) break;
	}

	if (fractParam.playMode && !Interface_data.keyframeMode) fclose(pFile_coordinates);

	delete fractParam.backgroundTexture;
	WriteLog("Released memory for background texture");
	delete fractParam.envmapTexture;
	WriteLog("Released memory for envmap texture");
	delete fractParam.lightmapTexture;
	WriteLog("Released memory for lightmap texture");

	if (fractParam.stereoEnabled)
	{
		delete secondEyeImage;
		delete[] stereoImage;
		WriteLog("Released memory for stereo image");
	}
	printf("Rendering finished\n");
	WriteLog("Rendering completely finished");
	isRendering = false;
}

void ThumbnailRender(const char *settingsFile, cImage *miniImage, int mode)
{
	printf("Rendering keyframe preview: %s\n", settingsFile);

	if (FileIfExists(settingsFile))
	{
		sParamRender fractParamLoaded;
		LoadSettings(settingsFile, fractParamLoaded, true);

		if (mode == 1)
		{
			KeepOtherSettings(&fractParamLoaded);
		}

		ThumbnailRender2(fractParamLoaded, miniImage);
	}
}

void ThumbnailRender2(sParamRender fractParamLoaded, cImage *miniImage)
{
	fractParamLoaded.image_width = miniImage->GetWidth();
	fractParamLoaded.image_height = miniImage->GetHeight();
	fractParamLoaded.doubles.resolution = 0.5 / fractParamLoaded.image_width;

	if (fractParamLoaded.fractal.formula == trig_DE || fractParamLoaded.fractal.formula == trig_optim || fractParamLoaded.fractal.formula == menger_sponge || fractParamLoaded.fractal.formula == kaleidoscopic
			|| fractParamLoaded.fractal.formula == tglad || fractParamLoaded.fractal.formula == smoothMandelbox) fractParamLoaded.fractal.analitycDE = true;
	else fractParamLoaded.fractal.analitycDE = false;
	fractParamLoaded.recordMode = false;
	fractParamLoaded.animMode = false;
	fractParamLoaded.quiet = true;
	fractParamLoaded.tileCount = 0;
	fractParamLoaded.noOfTiles = 1;

	RecalculateIFSParams(fractParamLoaded.fractal);
	CreateFormulaSequence(fractParamLoaded.fractal);

	miniImage->ClearImage();
	miniImage->SetImageParameters(fractParamLoaded.doubles.imageAdjustments);
	memcpy(Interface_data.palette, fractParamLoaded.palette, sizeof(sRGB)*256);

	LoadTextures(&fractParamLoaded);

	programClosed = false;
	isPostRendering = false;

	PlaceRandomLights(&fractParamLoaded, false);

	Render(fractParamLoaded, miniImage, NULL);

	delete fractParamLoaded.backgroundTexture;
	WriteLog("Released memory for background texture");
	delete fractParamLoaded.envmapTexture;
	WriteLog("Released memory for envmap texture");
	delete fractParamLoaded.lightmapTexture;
	WriteLog("Released memory for lightmap texture");
}
