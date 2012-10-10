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

//***************************** Main thread ****************************
//main rendering thread
void *MainThread(void *ptr)
{

	//getting parameters
	sParam *parametry;
	parametry = (sParam*) ptr;

	//initialising variables
	int start = parametry->start;
	start = (start / 16) * 16;
	int z = parametry->z;
	int thread_number = z;
	cImage *image = parametry->image;
	int progressive = parametry->progressive;
	int progressiveStart = parametry->progressiveStart;
	int width = image->GetWidth();
	int height = image->GetHeight();
	WriteLogDouble("Thread started", thread_number);

	//sRGB8 black8 = { 0, 0, 0 };
	sRGB16 black16 = { 0, 0, 0 };

	//printf("Thread #%d started\n", z + 1);
	sParamRender param = parametry->param;

	int tiles = param.noOfTiles;
	int tile = param.tileCount;
	double dist_thresh = param.doubles.dist_thresh;
	double DE_factor = param.doubles.DE_factor;
	CVector3 vp = param.doubles.vp;
	double min_y = param.doubles.min_y;
	double max_y = param.doubles.max_y;
	double resolution = param.doubles.resolution / tiles;
	double zoom = param.doubles.zoom;
	double alfa = param.doubles.alfa;
	double beta = param.doubles.beta;
	double gamma = param.doubles.gamma;
	double persp = param.doubles.persp;
	double quality = param.doubles.quality;
	bool shadow = param.shadow;
	bool global_ilumination = param.global_ilumination;
	bool fastGlobalIllumination = param.fastGlobalIllumination;
	int AO_quality = param.globalIlumQuality;


	bool fractColor = param.imageSwitches.coloringEnabled;
	cTexture *envmap_texture = param.envmapTexture;
	cTexture *lightmap_texture = param.lightmapTexture;

	//preparing rotation matrix
	CRotationMatrix mRot;
	mRot.RotateZ(alfa);
	mRot.RotateX(beta);
	mRot.RotateY(gamma);

	//preparing base vectors
	CVector3 vector, vDelta;
	CVector3 baseX(1, 0, 0);
	CVector3 baseY(0, 1, 0);
	CVector3 baseZ(0, 0, 1);

	baseX = mRot.RotateVector(baseX);
	baseY = mRot.RotateVector(baseY);
	baseZ = mRot.RotateVector(baseZ);

	//shadow direction vector
	vector.x = cos(param.doubles.mainLightAlfa - 0.5 * M_PI) * cos(-param.doubles.mainLightBeta);
	vector.y = sin(param.doubles.mainLightAlfa - 0.5 * M_PI) * cos(-param.doubles.mainLightBeta);
	vector.z = sin(-param.doubles.mainLightBeta);
	vDelta = mRot.RotateVector(vector);

	//raymarching step depends on resolution
	double stepY = resolution;
	double delta = resolution * zoom;

	//distance buffer
	double *distanceBuff = new double[10002];
	double *stepBuff = new double[10002];
	double *distanceBuffRefl = new double[10002];
	double *stepBuffRefl = new double[10002];
	int buffCount = 0;

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
			vectorsAround[counter].alfa = a;
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
			if(counter>=10000) break;
		}
		if(counter>=10000) break;
	}
	if (counter == 0)
	{
		counter = 1;
		vectorsAround[0].alfa = 0;
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
	double y_scan_start = (0.00001 - 1.0) / persp;
	min_y = param.doubles.viewDistanceMin/zoom - 1.0/persp;
	max_y = param.doubles.viewDistanceMax/zoom - 1.0/persp;

	//distance value after raymarching
	double last_distance = 0;

	//parameters for iteration functions
	sFractal calcParam = param.fractal;
	bool max_iter;

	double search_accuracy = 0.01;
	double search_limit = 1.0 - search_accuracy;

	WriteLogDouble("All vectors and matrices prepared", thread_number);

	enumPerspectiveType perspectiveType = param.perspectiveType;
	double fov = param.doubles.persp;

	double tileXOffset = (tile % tiles);
	double tileYOffset = (tile / tiles);

	//2-pass loop (for multi-threading)
	for (int pass = 0; pass < 2; pass++)
	{
		if (pass == 1) start = 0;

		//main loop for z values
		for (z = start; z <= height - progressive; z += progressive)
		{
			//checking if some another thread is not rendering the same z
			if (parametry->thread_done[z] == 0)
			{
				//giving information for another threads that this thread renders this z value
				parametry->thread_done[z] = thread_number + 1;

				//WriteLogDouble("Started rendering line", z);

				//main loop for x values
				for (int x = 0; x <= width - progressive; x += progressive)
				{
					//checking if program was not closed
					if (programClosed)
					{
						delete[] vectorsAround;
						delete[] distanceBuff;
						delete[] stepBuff;
						delete[] distanceBuffRefl;
						delete[] stepBuffRefl;
						return NULL;
					}

					if (progressive < progressiveStart && x % (progressive * 2) == 0 && z % (progressive * 2) == 0) continue;

					//------------- finding fractal surface ---------------------------

					//calculating starting point
					double y_start = max_y;
					if (y_scan_start > min_y) min_y = y_scan_start;

					double x2,z2;
					bool hemisphereCut = false;
					if (perspectiveType == fishEye || perspectiveType == equirectangular)
					{
						x2 = ((double) x / width / tiles - 0.5 + tileXOffset/tiles) * aspectRatio;
						z2 = ((double) z / height / tiles  - 0.5 + tileYOffset/tiles);
						if(param.fishEyeCut && sqrt(x2*x2 + z2*z2) > 0.5 / fov)
						{
								hemisphereCut = true;
						}
						x2*=M_PI;
						z2*=M_PI;
					}
					else
					{
						x2 = ((double) x / width / tiles - 0.5 + tileXOffset/tiles) * zoom * aspectRatio;
						z2 = ((double) z / height / tiles - 0.5 + tileYOffset/tiles) * zoom;
					}

					//preparing variables
					bool found = false;
					double dist = delta;
					double stepYpersp = delta;
					counter = 0;
					int binary_step = 0;
					bool binary = false;
					buffCount = 0;

					CVector3 viewVectorStart(0, 0, 0);
					CVector3 viewVectorEnd(0, 0, 0);

					if (perspectiveType == fishEye || perspectiveType == equirectangular)
					{
						min_y = 1e-15;
						max_y = 100;
					}

					double correction = 0.1;
					double lastCorrection = 0.1;

					double lastStepY = 0;
					bool firstLoop = true;
					double lastDist = 0;

					calcParam.specialColour = 0;
					int specialColour = 0;

					//main loop for y values (depth)
					for (double y = min_y; y < max_y; y += stepYpersp)
					{
						//recheck threads
						if (parametry->thread_done[z] == thread_number + 1)
						{

							//perspective factor (wsp = factor in Polish :-)
							double wsp_persp = 1.0 + y * persp;

							if (param.fractal.constantDEThreshold)
							{
								dist_thresh = quality;
								if (perspectiveType == fishEye || perspectiveType == equirectangular) search_accuracy = 0.01 * 2.0 * y * resolution / quality * fov;
								else	search_accuracy = 0.01* zoom * stepY * wsp_persp  / quality;
								if(search_accuracy>0.1) search_accuracy = 0.1;
								search_limit = 1.0 - search_accuracy;
							}
							else
							{
								//recalculating dynamic DE threshold
								if (perspectiveType == fishEye || perspectiveType == equirectangular)
								{
									dist_thresh = 2.0 * y * resolution / quality * fov;
									search_accuracy = 0.01*quality;
									search_limit = 1.0 - search_accuracy;
								}
								else
								{
									dist_thresh = zoom * stepY * wsp_persp / quality;
									search_accuracy = 0.01*quality;
									search_limit = 1.0 - search_accuracy;
								}
							}
							if (wsp_persp > 0) //checking for sure :-)
							{
								//DE steps counter
								counter++;

								//rotate coordinate system from screen to fractal coordinates and perspective projection
								CVector3 point = Projection3D(CVector3(x2, y, z2), vp, mRot, perspectiveType, fov, zoom);

								if (counter == 1)
								{
									viewVectorStart = point;
								}

								//calculate opacity

								calcParam.doubles.detailSize = dist_thresh;
								dist = CalculateDistance(point, calcParam, &max_iter);

								distanceBuff[buffCount]=dist;

								//it is for situation when DE is calculated with very big error
								if (!param.fractal.dynamicDEcorrection && dist > 5.0 / DE_factor)
								{
									dist = 5.0 / DE_factor;
								}

								//if maxiter then distance is zero
								if (max_iter)
								{
									dist = 0;
								}

								//iteration threshold mode
								if (param.fractal.iterThresh)
								{
									if (dist < dist_thresh && !max_iter)
									{
										dist = dist_thresh * 1.01;
									}
								}

								//if distance is less than distance threshold
								if (dist < dist_thresh && !binary)
								{

									//if shearching goes to far switching to binary searching mode
									if ((dist < search_limit * dist_thresh)) //1% accuracy of distance
									{
										binary = true;
										if (dist < 0.1 * dist_thresh) Missed_DE_counter++;
									}
									else //if distance is OK, end of surface searching
									{
										found = true;
										y_start = y;
										last_distance = dist;
										//printf("found in distance %e and count %d\n", dist, counter);
										break;
									}
								}

								if (counter > 10000) //if something is wrong (elimination endless loop)

								{
									//printf("stepYpersp = %e, dist = %e, y = %f\n", stepYpersp, dist*10.0, y);
									found = true;
									y_start = y;
									last_distance = dist;
									break;
								}

								//step in DE mode
								if (!binary)
								{
									if (perspectiveType == fishEye || perspectiveType == equirectangular) stepYpersp = dist * DE_factor;
									else stepYpersp = (dist - 0.5 * dist_thresh) / zoom * DE_factor;

									stepYpersp = stepYpersp * (1.0 - Random(1000)/10000.0);

									stepBuff[buffCount] = stepYpersp * zoom;
									buffCount++;
								}

								//step in binary searching mode

								else
								{
									binary_step++;

									//if distance is to low then cancel last step
									if (dist < dist_thresh * search_limit)
									{
										y -= stepYpersp;
									}

									//go forward half step
									stepYpersp = stepYpersp / 2.0;

									//ending binary shearching
									if (binary_step > 20 || (dist < dist_thresh && dist > dist_thresh * search_limit))
									{
										found = true;
										y_start = y;

										last_distance = dist;

										break;
									}
								}

								//DE fractor - dynamic correction
								if (!firstLoop && !binary)
								{
									double deltaDist = lastDist - dist;

									double deltaY = (lastStepY) * zoom / DE_factor / correction + 0.5 * dist_thresh;
									DE_counterForDEerror++;
									if(deltaDist > deltaY || deltaDist == 0)
									{
										DEerror += fabs((deltaDist - deltaY) / deltaY);
									}
									else
									{
										DEerror += fabs((deltaY - deltaDist) / deltaDist);
									}

									if (param.fractal.dynamicDEcorrection)
									{
										CVector3 vn = CalculateNormals(&param, &calcParam, point, dist * 0.1);
										CVector3 direction = point - viewVectorStart;
										direction.Normalize();
										double directionFactor = vn.Dot(direction) * (-1.0);

										double newCorrection = fabs(deltaY / deltaDist * directionFactor);
										//if(thread_number == 0)
										//{
										//	printf("tresh: %f, dist: %f, ddist: %f, dy: %f, newCorr: %f, direct: %f, last: %f\n", dist_thresh, dist, deltaDist, deltaY, newCorrection, directionFactor, correction);
										//}
										if (newCorrection > 1.0) newCorrection = 1.0;
										if (newCorrection < 0.01) newCorrection = 0.01;

										if (newCorrection < lastCorrection) correction = newCorrection * (newCorrection * newCorrection) / (correction * correction) * 0.5;
										else correction = correction + (newCorrection - lastCorrection) * lastCorrection * 0.2;

										//if(fabs(directionFactor) < 0.1) correction = lastCorrection;

										if (correction > 1.0) correction = 1.0;
										if (correction < 0.01) correction = 0.01;
									}
									else
									{
										correction = 1.0;
									}

									//printf("%f ",correction);
								}
								else
								{
									if (param.fractal.dynamicDEcorrection)
									{
										correction = 1.0;
									}
									else
									{
										correction = 1.0;
									}
								}

								if (lastDist - dist > 0)
								{
									firstLoop = false;
								}

								stepYpersp *= correction;

								lastDist = dist;
								lastStepY = stepYpersp;
								lastCorrection = correction;

								if (hemisphereCut)
								{
									found = false;
									y_start = y;
									last_distance = dist;
									break;
								}
							}
						}
					} //next y

					//if(thread_number == 0) printf("next y\n");

					DE_counter += counter;
					Pixel_counter++;
					//counters for drawing histogram
					int counter2 = counter / 4;
					if (counter2 < 256) histogram2[counter2]++;
					else histogram2[255]++;

					if(calcParam.specialColour != 0) specialColour = calcParam.specialColour;

					sComplexImage pixelData;
					memset(&pixelData,0,sizeof(sComplexImage));

					CVector3 lightVector = vDelta;

					double y = y_start;
					CVector3 point = Projection3D(CVector3(x2, y, z2), vp, mRot, perspectiveType, fov, zoom);
					double wsp_persp = 1.0 + y * persp;

					viewVectorEnd = point;
					CVector3 viewVector = viewVectorEnd - viewVectorStart;
					viewVector.Normalize();

					//delta for all shading algorithms depended on depth and resolution (dynamic shaders resolution)
					if (perspectiveType == fishEye || perspectiveType == equirectangular)
					{
						delta = resolution * y * fov;
						wsp_persp = 2.0 * y;
						zoom = param.doubles.zoom = 1.0;
					}
					else
					{
						delta = resolution * zoom * wsp_persp;
					}

					//if fractal surface was found
					if (found)
					{
						//-------------------- SHADING ----------------------

						//initial values
						sShaderOutput shadowOutput = { 1.0, 1.0, 1.0 };
						sShaderOutput AO;
						sRGB16 oldAO = {0,0,0};
						if(!image->IsLowMemMode()) oldAO = image->GetPixelAmbient(x, z);
						AO.R = oldAO.R / 4096.0;
						AO.G = oldAO.G / 4096.0;
						AO.B = oldAO.B / 4096.0;
						sShaderOutput shade;
						sShaderOutput shadeAux;
						sShaderOutput shadeAuxSpec;
						sShaderOutput envMapping;
						int colorIndex = 0;

						//normal vector
						CVector3 vn = CalculateNormals(&param, &calcParam, point, dist_thresh);

						//********* calculate hard shadow
						if (shadow) shadowOutput = MainShadow(&param, &calcParam, point, lightVector, wsp_persp, dist_thresh);

						//******* calculate global illumination (ambient occlusion)
						if (global_ilumination)
						{
							//ambient occlusion based on orbit traps
							if (fastGlobalIllumination)
							{
								//AO = FastAmbientOcclusion(&calcParam, point);
								AO = FastAmbientOcclusion2(&calcParam, point, vn, dist_thresh, param.doubles.fastAoTune, param.globalIlumQuality);
							}
							//ambient occlusion based on many rays in spherical directions
							else
							{
								AO = AmbientOcclusion(&param, &calcParam, point, wsp_persp, dist_thresh, last_distance, vectorsAround, vectorsCount);
							}
						}

						//calculate shading based on angle of incidence
						shade = MainShading(vn, lightVector);

						//calculate specular reflection effect
						sShaderOutput specular = MainSpecular(vn, lightVector, viewVector);

						//calculate shading from auxiliary lights
						shadeAux = AuxLightsShader(&param, &calcParam, point, vn, viewVector, wsp_persp, dist_thresh, &shadeAuxSpec);

						//calculate environment mapping reflection effect
						envMapping = EnvMapping(vn, viewVector, envmap_texture);

						//coloured surface of fractal
						if (fractColor)	colorIndex = SurfaceColour(&calcParam, point);
						if (specialColour != 0) colorIndex = specialColour*256;

						//--------------- raytraced reflection
						if(param.imageSwitches.raytracedReflections)
						{
							CVector3 pointTemp = point;
							CVector3 viewTemp = viewVector;
							CVector3 vnTemp = vn;
							bool maxIterTemp;

							double fog_visibility = pow(10, param.doubles.imageAdjustments.fogVisibility / 10 - 2.0)*zoom;

							envMapping.R = envMapping.G = envMapping.B = 0;
							sShaderOutput shadeBuff[10];
							sShaderOutput shadowBuff[10];
							sShaderOutput specularBuff[10];
							sShaderOutput colorBuff[10];
							sShaderOutput ambientBuff[10];
							sShaderOutput auxLightsBuff[10];
							sShaderOutput auxSpecBuff[10];
							sShaderOutput volFogBuff[10];
							double fogDensityBuff[10];
							double fogBuff[10];
							sShaderOutput finishBackgroud;
							double glowBuff1 = 0.0;
							int numberOfReflections = -1;
							int maxReflections = param.reflectionsMax;
							double totalFog = 0;
							for(int i=0; i<maxReflections; i++)
							{
								viewTemp = viewTemp - vnTemp * viewTemp.Dot(vnTemp)*2.0;
								double distTemp = dist_thresh;
								bool surfaceFound = false;
								double glowBuff2 = 0.0;
								bool binary = false;
								double step = dist_thresh;
								int binarySearchCount = 0;
								int buffCountRefl = 0;
								for(double scan = dist_thresh*2.0; scan<100.0; scan += step)
								{
									glowBuff2++;
									CVector3 pointScan = pointTemp + viewTemp * scan;
									distTemp = CalculateDistance(pointScan, calcParam, &maxIterTemp);
									specialColour = calcParam.specialColour;
									if(!binary)
									{
										distanceBuffRefl[buffCountRefl] = distTemp;
										stepBuffRefl[buffCountRefl] = step;
										buffCountRefl++;
										if(buffCountRefl>=10000) break;
									}
									if((distTemp < dist_thresh && distTemp > dist_thresh * search_limit) || binarySearchCount > 20)
									{
										surfaceFound = true;
										pointTemp = pointScan;
										vnTemp = CalculateNormals(&param, &calcParam, pointTemp, dist_thresh);
										shadeBuff[i] = MainShading(vnTemp, lightVector);
										specularBuff[i] = MainSpecular(vnTemp, lightVector, viewTemp);
										shadowBuff[i].R = shadowBuff[i].G = shadowBuff[i].B = 1.0;
										if (shadow) shadowBuff[i] = MainShadow(&param, &calcParam, pointTemp, lightVector, wsp_persp, dist_thresh);
										ambientBuff[i].R = ambientBuff[i].G = ambientBuff[i].B = 0;
										if (global_ilumination)
										{
											if (fastGlobalIllumination)
											{
												ambientBuff[i] = FastAmbientOcclusion2(&calcParam, pointTemp, vnTemp, dist_thresh, param.doubles.fastAoTune, param.globalIlumQuality);
											}
											else
												ambientBuff[i] = AmbientOcclusion(&param, &calcParam, pointTemp, wsp_persp, dist_thresh, last_distance, vectorsAround, vectorsCount);
										}

										auxLightsBuff[i] = AuxLightsShader(&param, &calcParam, pointTemp, vnTemp, viewTemp, wsp_persp, dist_thresh, &auxSpecBuff[i]);

										int colorIndexTemp = SurfaceColour(&calcParam, pointTemp);
										if (specialColour != 0) colorIndexTemp = specialColour*256;
										sRGB color;
										color.R = color.G = color.B = 256;
										if (param.imageSwitches.coloringEnabled)
										{
											int color_number;
											if(colorIndexTemp>=248*256)
											{
												color_number = colorIndexTemp;
											}
											else
											{
												color_number = (int) (colorIndexTemp * param.doubles.imageAdjustments.coloring_speed + 256 * param.doubles.imageAdjustments.paletteOffset) % 65536;
											}
											color = image->IndexToColour(color_number);
										}
										colorBuff[i].R = color.R/256.0;
										colorBuff[i].G = color.G/256.0;
										colorBuff[i].B = color.B/256.0;

										totalFog += scan;
										fogBuff[i] = totalFog;;

										numberOfReflections = i;
										break;
									}

									if(distTemp < dist_thresh * search_limit)
									{
										binary = true;
										scan-=step;
									}
									if(!binary)
										step = (distTemp-0.5*dist_thresh)*DE_factor;
									else
									{
										step = step * 0.5;
										binarySearchCount++;
									}
								}
								glowBuff1 += glowBuff2 * pow(param.doubles.imageAdjustments.reflect, i + 1.0);

								double density = 0;
								volFogBuff[i] = VolumetricFog(&param, buffCountRefl, distanceBuffRefl, stepBuffRefl, &density);
								fogDensityBuff[i] = density / (1.0 + density);

								if(!surfaceFound)
								{
									finishBackgroud = TexturedBackground(&param, viewTemp);
									break;
								}
							}

							double reflect = param.doubles.imageAdjustments.reflect;

							bool wasMaxReflections = true;
							if(numberOfReflections<maxReflections-1)
							{
								wasMaxReflections = false;
								if(param.imageSwitches.fogEnabled)
								{
									envMapping.R = param.effectColours.fogColor.R / 65536.0 * reflect;
									envMapping.G = param.effectColours.fogColor.G / 65536.0 * reflect;
									envMapping.B = param.effectColours.fogColor.B / 65536.0 * reflect;
								}
								else
								{
									envMapping.R = finishBackgroud.R * reflect;
									envMapping.G = finishBackgroud.G * reflect;
									envMapping.B = finishBackgroud.B * reflect;
								}

								double fogN = 1.0 - fogDensityBuff[0];
								if(fogN<0) fogN = 0;
								envMapping.R = envMapping.R * fogN + volFogBuff[0].R/65536.0 * fogDensityBuff[0];
								envMapping.G = envMapping.G * fogN + volFogBuff[0].G/65536.0 * fogDensityBuff[0];
								envMapping.B = envMapping.B * fogN + volFogBuff[0].B/65536.0 * fogDensityBuff[0];
							}

							for(int i=numberOfReflections; i>=0; i--)
							{
								sShaderOutput reflectTemp;
								double mainIntensity = param.doubles.imageAdjustments.mainLightIntensity * param.doubles.imageAdjustments.directLight;
								reflectTemp.R = (shadeBuff[i].R * param.doubles.imageAdjustments.shading + 1.0 - param.doubles.imageAdjustments.shading)
										* shadowBuff[i].R * (1.0 - param.doubles.imageAdjustments.ambient) * colorBuff[i].R * param.effectColours.mainLightColour.R / 65536.0 * mainIntensity
										+ (param.doubles.imageAdjustments.ambient + ambientBuff[i].R + auxLightsBuff[i].R) * colorBuff[i].R;
								reflectTemp.G = (shadeBuff[i].G * param.doubles.imageAdjustments.shading + 1.0 - param.doubles.imageAdjustments.shading)
										* shadowBuff[i].G * (1.0 - param.doubles.imageAdjustments.ambient) * colorBuff[i].G * param.effectColours.mainLightColour.G / 65536.0 * mainIntensity
										+ (param.doubles.imageAdjustments.ambient + ambientBuff[i].G + auxLightsBuff[i].G) * colorBuff[i].G;
								reflectTemp.B = (shadeBuff[i].B * param.doubles.imageAdjustments.shading + 1.0 - param.doubles.imageAdjustments.shading)
										* shadowBuff[i].B * (1.0 - param.doubles.imageAdjustments.ambient) * colorBuff[i].B * param.effectColours.mainLightColour.B / 65536.0 * mainIntensity
										+ (param.doubles.imageAdjustments.ambient + ambientBuff[i].B + auxLightsBuff[i].B) * colorBuff[i].B;

								double fogTransparency = 1.0;
								if(param.imageSwitches.fogEnabled)
								{
									double fog = fogBuff[i] / fog_visibility;
									if (fog > 1.0) fog = 1.0;
									if (fog < 0) fog = 0;
									double a = fog;
									double aN = 1.0 - a;
									reflectTemp.R = (reflectTemp.R * aN + param.effectColours.fogColor.R / 65536.0 * a);
									reflectTemp.G = (reflectTemp.G * aN + param.effectColours.fogColor.G / 65536.0 * a);
									reflectTemp.B = (reflectTemp.B * aN + param.effectColours.fogColor.B / 65536.0 * a);
									fogTransparency = aN;
								}

								if(wasMaxReflections && i == numberOfReflections)
								{
									envMapping = reflectTemp;
								}

								envMapping.R = reflectTemp.R + (reflect)*envMapping.R
										+ (specularBuff[i].R * shadowBuff[i].R * param.effectColours.mainLightColour.R / 65536.0 * mainIntensity * param.doubles.imageAdjustments.specular
										+ auxSpecBuff[i].R * param.doubles.imageAdjustments.specular)
										* reflect * fogTransparency;
								envMapping.G = reflectTemp.G + (reflect)*envMapping.G
										+ (specularBuff[i].G * shadowBuff[i].G * param.effectColours.mainLightColour.G / 65536.0 * mainIntensity * param.doubles.imageAdjustments.specular
										+ auxSpecBuff[i].G * param.doubles.imageAdjustments.specular)
										* reflect * fogTransparency;
								envMapping.B = reflectTemp.B + (reflect)*envMapping.B
										+ (specularBuff[i].B * shadowBuff[i].B * param.effectColours.mainLightColour.B / 65536.0 * mainIntensity * param.doubles.imageAdjustments.specular
										+ auxSpecBuff[i].B * param.doubles.imageAdjustments.specular)
										* reflect * fogTransparency;

								double fogN = 1.0 - fogDensityBuff[i];
								if(fogN<0) fogN = 0;
								envMapping.R = envMapping.R * fogN + volFogBuff[i].R/65536.0 * fogDensityBuff[i];
								envMapping.G = envMapping.G * fogN + volFogBuff[i].G/65536.0 * fogDensityBuff[i];
								envMapping.B = envMapping.B * fogN + volFogBuff[i].B/65536.0 * fogDensityBuff[i];
							}
							double glow = glowBuff1 * param.doubles.imageAdjustments.glow_intensity / 512.0;
							double glowN = 1.0 - glow;
							if (glowN < 0.0) glowN = 0.0;
							double glowR = (param.effectColours.glow_color1.R * glowN /65536.0 + param.effectColours.glow_color2.R * glow /65536.0);
							double glowG = (param.effectColours.glow_color1.G * glowN /65536.0 + param.effectColours.glow_color2.G * glow /65536.0);
							double glowB = (param.effectColours.glow_color1.B * glowN /65536.0 + param.effectColours.glow_color2.B * glow /65536.0);

							envMapping.R = envMapping.R * glowN + glowR * glow;
							envMapping.G = envMapping.G * glowN + glowG * glow;
							envMapping.B = envMapping.B * glowN + glowB * glow;

							if(envMapping.R>10.0) envMapping.R = 10.0;
							if(envMapping.G>10.0) envMapping.G = 10.0;
							if(envMapping.B>10.0) envMapping.B = 10.0;
						}
						//---------- end of raytraced reflection

						unsigned short shadow16 = shadowOutput.R * 4096.0;
						sRGB16 globalLight = { AO.R * 4096.0, AO.G * 4096.0, AO.B * 4096.0 };
						unsigned short shade16 = shade.R * 4096.0;
						sRGB16 shadeAux16 = { shadeAux.R * 4096.0, shadeAux.G * 4096.0, shadeAux.B * 4096.0 };
						sRGB16 specularAux16 = { shadeAuxSpec.R * 4096.0, shadeAuxSpec.G * 4096.0, shadeAuxSpec.B * 4096.0 };
						sRGB16 reflection = { envMapping.R * 256.0, envMapping.G * 256.0, envMapping.B * 256.0 };

						if (!image->IsLowMemMode())
						{
							image->PutPixelShadow(x, z, shadow16);
							image->PutPixelAmbient(x, z, globalLight);
							image->PutPixelShading(x, z, shade16);
							image->PutPixelSpecular(x, z, specular.R * 4096.0);
							image->PutPixelAuxLight(x, z, shadeAux16);
							image->PutPixelAuxSpecular(x, z, specularAux16);
							image->PutPixelReflect(x, z, reflection);
							image->PutPixelBackground(x, z, black16);
						}

						image->PutPixelColor(x, z, colorIndex);

						if (perspectiveType == fishEye || perspectiveType == equirectangular)
						{
							image->PutPixelZBuffer(x, z, y*1.0e12);
						}
						else
						{
							image->PutPixelZBuffer(x, z, y);
						}
						pixelData.shadowsBuf16 = shadow16;
						pixelData.ambientBuf16 = globalLight;
						pixelData.shadingBuf16 = shade16;
						pixelData.specularBuf16 = specular.R * 4096.0;
						pixelData.auxLight = shadeAux16;
						pixelData.auxSpecular = specularAux16;
						pixelData.reflectBuf16 = reflection;
						pixelData.backgroundBuf16 = black16;

					}//end if found

					else
					{
						sShaderOutput background = { 0.0, 0.0, 0.0 };

						//------------- render 3D background
						background = TexturedBackground(&param, viewVector);

						sRGB16 background16 = { background.R * 65536.0, background.G * 65536.0, background.B * 65536.0 };
						pixelData.backgroundBuf16 = background16;

						if (!image->IsLowMemMode())
						{
							image->PutPixelBackground(x, z, background16);
							image->PutPixelAmbient(x, z, black16);
							image->PutPixelAuxLight(x, z, black16);
							image->PutPixelAuxSpecular(x, z, black16);
							image->PutPixelColor(x, z, 0);
							image->PutPixelReflect(x, z, black16);
							image->PutPixelShading(x, z, 0);
							image->PutPixelShadow(x, z, 0);
							image->PutPixelSpecular(x, z, 0);
						}

						image->PutPixelZBuffer(x, z, 1e20);
					}

					//drawing transparent glow
					if(!image->IsLowMemMode()) image->PutPixelGlow(x, z, counter);
					pixelData.glowBuf16 = counter;

					//************* volumetric fog / light
					sShaderOutput volFog;
					volFog.R = volFog.G = volFog.B = 0.0;
					if (!param.imageSwitches.iterFogEnabled)
					{
						if (param.imageSwitches.volumetricLightEnabled)
						{
							volFog = VolumetricLight(&param, &calcParam, CVector3(x2, y, z2), y, min_y, last_distance, zoom, lightVector);
						}
						else
						{
							double density;
							volFog = VolumetricFog(&param, buffCount, distanceBuff, stepBuff, &density);
							pixelData.fogDensity16 = 65535 * density / (1.0 + density);
						}
					}
					else
					{
						//iteration fog

						unsigned short alpha2 = 0;
						float zBuf = image->GetPixelZBuffer(x,z);
						unsigned short colorIndex = image->GetPixelColor(x,z);
						double fog_visibility = pow(10, param.doubles.imageAdjustments.fogVisibility / 10 - 2.0);
						double fog_visibility_front = pow(10,  param.doubles.imageAdjustments.fogVisibilityFront / 10 - 2.0) - 10.0;
						pixelData.volumetricLight.R = 0;
						pixelData.volumetricLight.G = 0;
						pixelData.volumetricLight.B = 0;
						pixelData.fogDensity16 = 0;
						sRGB16 oldPixel16 = image->CalculatePixel(pixelData, alpha2, zBuf, colorIndex, fog_visibility, fog_visibility_front);
						double density;
						volFog = IterationFog(&param, &calcParam, CVector3(x2, y, z2), y, min_y, last_distance, zoom, lightVector, found, &density, vectorsAround, vectorsCount, oldPixel16);
						pixelData.fogDensity16 = 65535;
						//end of iteration fog
					}

					if(volFog.R>65535.0) volFog.R = 65535.0;
					if(volFog.G>65535.0) volFog.G = 65535.0;
					if(volFog.B>65535.0) volFog.B = 65535.0;

					pixelData.volumetricLight.R = volFog.R;
					pixelData.volumetricLight.G = volFog.G;
					pixelData.volumetricLight.B = volFog.B;

					if(!image->IsLowMemMode())
					{
						image->PutPixelVolumetricFog(x, z, pixelData.volumetricLight);
						image->PutPixelFogDensity(x, z, pixelData.fogDensity16);
					}

					if(image->IsLowMemMode())
					{
						unsigned short alpha2 = 0;
						float zBuf = image->GetPixelZBuffer(x,z);
						unsigned short colorIndex = image->GetPixelColor(x,z);
						double fog_visibility = pow(10, param.doubles.imageAdjustments.fogVisibility / 10 - 2.0);
						double fog_visibility_front = pow(10,  param.doubles.imageAdjustments.fogVisibilityFront / 10 - 2.0) - 10.0;
						sRGB16 newPixel16 = image->CalculatePixel(pixelData, alpha2, zBuf, colorIndex, fog_visibility, fog_visibility_front);
						image->PutPixelImage(x,z,newPixel16);
						image->PutPixelAlpha(x,z,alpha2);
					}

				}//next x

				if(!image->IsLowMemMode()) image->Squares(z, progressive);

				(*parametry->done)++;

				double progressiveDone, percent_done;
				if (progressive == progressiveStart) progressiveDone = 0;
				else progressiveDone = 0.25 / (progressive * progressive);

				if (progressiveStart == 1)
				{
					percent_done = ((double) *parametry->done / height) * 100.0;
				}
				else
				{
					percent_done = (((double) *parametry->done / height) * 3.0 / 4.0 / progressive + progressiveDone) * 100.0;
				}

				if(param.noOfTiles > 1)
				{
					percent_done = (param.tileCount + percent_done/100.0) / (param.noOfTiles * param.noOfTiles)*100.0;
				}
				if(Interface_data.animMode)
				{
					percent_done = (param.fractal.frameNo - param.startFrame + percent_done/100.0) / (param.endFrame - param.startFrame + 1) * 100.0;
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
				if (*parametry->done == height - 1)
				{
					double avg_N = (double) N_counter / Loop_counter;
					double avg_DE = (double) DE_counter / Pixel_counter;
					double avgMissedDE = (double) Missed_DE_counter / Pixel_counter * 100.0;
					printf("Average N = %f, Average DE steps = %f, Missed DE %.3f%%\n", avg_N, avg_DE, avgMissedDE);
				}

				//WriteLogDouble("Rendering line finished", z);
			}//end if thread done

		}//next z
	}// next pass
	WriteLogDouble("Thread finished", thread_number);
	delete[] vectorsAround;
	delete[] distanceBuff;
	delete[] stepBuff;
	delete[] distanceBuffRefl;
	delete[] stepBuffRefl;

	return 0;
}

//************************** Render **************************************
void Render(sParamRender param, cImage *image, GtkWidget *outputDarea)
{

	programClosed = false;
	//getting image resolution from parameters
	int width = image->GetWidth();
	int height = image->GetHeight();

	if (!clSupport->IsReady())
	{
		//turn off refreshing if resolution is very low
		bool bo_refreshing = true;

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
		GThread **Thread = new GThread *[NR_THREADS + 1];
		GError **err = new GError *[NR_THREADS + 1];
		sParam *thread_param = new sParam[NR_THREADS];

		int progressiveStart = 8;
		if (param.recordMode || noGUI) progressiveStart = 1;
		image->progressiveFactor = progressiveStart;

		int refresh_index = 0;
		int refresh_skip = 1;

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
			}
			WriteLog("Thread data prepared");

			//running rendering threads in background
			for (int i = 0; i < NR_THREADS; i++)
			{
				//sending some parameters to thread
				thread_param[i].start = i * height / NR_THREADS;
				thread_param[i].z = i;
				thread_param[i].param = param;
				thread_param[i].image = image;
				thread_param[i].progressive = progressive;
				thread_param[i].progressiveStart = progressiveStart;
				thread_param[i].done = &done;
				thread_param[i].thread_done = thread_done;
				//creating thread
				Thread[i] = g_thread_create((GThreadFunc) MainThread, &thread_param[i], TRUE, &err[i]);
				WriteLogDouble("Thread created", i);
				//printf("Rendering thread #%d created\n", i + 1);
			}

			//refresh GUI
			if (!noGUI && bo_refreshing)
			{
				if (image->IsPreview())
				{
					while (gtk_events_pending())
						gtk_main_iteration();
				}

				//refreshing image and histograms during rendering
				while (done < height / progressive - 1 && !programClosed)
				{
					g_usleep(100000);
					if (progressive < progressiveStart)
					{
						refresh_index++;

						if (refresh_index >= refresh_skip)
						{
							double refreshStartTime = real_clock();

							if (image->IsPreview())
							{
								param.SSAOEnabled = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkSSAOEnabled));
								if (param.SSAOEnabled && !image->IsLowMemMode())
								{
									param.SSAOQuality = gtk_adjustment_get_value(GTK_ADJUSTMENT(Interface.adjustmentSSAOQuality));
									PostRendering_SSAO(image, param.doubles.persp, param.SSAOQuality / 2, param.perspectiveType, param.quiet);
									WriteLog("SSAO rendered");
								}
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

							double refreshEndTime = real_clock();
							//printf("RefreshTime: %f\n", refreshEndTime - refreshStartTime);
							refresh_skip = (refreshEndTime - refreshStartTime) * 50;
							refresh_index = 0;
							WriteLogDouble("Image refreshed", refreshEndTime - refreshStartTime);
						}
					}

					if (outputDarea != NULL)
					{
						//progress bar
						char progressText[1000];
						double progressiveDone, percent_done;
						if (progressive == progressiveStart) progressiveDone = 0;
						else progressiveDone = 0.25 / (progressive * progressive);
						if (progressiveStart == 1)
						{
							percent_done = ((double) done / height) * 100.0;
						}
						else
						{
							percent_done = (((double) done / height) * 3.0 / 4.0 / progressive + progressiveDone) * 100.0;
						}

						if(param.noOfTiles > 1)
						{
							percent_done = (param.tileCount + percent_done/100.0) / (param.noOfTiles * param.noOfTiles)*100.0;
						}
						if(Interface_data.animMode)
						{
							percent_done = (param.fractal.frameNo - param.startFrame + percent_done/100.0) / (param.endFrame - param.startFrame + 1) * 100.0;
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
					//refreshing GUI
					if (image->IsPreview())
					{
						while (gtk_events_pending())
							gtk_main_iteration();
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
			if (programClosed) break;
		} //next progress
		printf("\n");

		//refreshing image

		//*** postprocessing

		if (param.SSAOEnabled && !programClosed)
		{
			PostRendering_SSAO(image, param.doubles.persp, param.SSAOQuality, param.perspectiveType, param.quiet);
			WriteLog("SSAO rendered");
		}
		image->CompileImage();
		WriteLog("Image compiled");
		if (param.DOFEnabled && !programClosed)
		{
			double DOF_focus = pow(10, param.doubles.DOFFocus / 10.0 - 2.0) - 1.0 / param.doubles.persp;
			PostRendering_DOF(image, param.doubles.DOFRadius * width / 1000.0, DOF_focus, param.doubles.persp);
			WriteLog("DOF rendered");
		}
		PostRenderingLights(image, &param);
		WriteLog("Lights rendered");

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
	}
	else
	{
#ifdef CLSUPPORT
		printf("OpenCL rendering\n");
		sClFractal clFractal;
		sClParams clParams;
		clSupport->SetSize(image->GetWidth(), image->GetHeight());

		sClInBuff *inCLBuff = clSupport->GetInBuffer1();
		Params2Cl(&param, &clParams, &clFractal, inCLBuff);

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
		clParams.AmbientOcclusionNoOfVectors = counter;
		printf("Ambient occlusion counter %d\n", counter);

		clSupport->SetParams(clParams, clFractal, param.fractal.formula);
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

	mainImage.SetLowMem(noGUIdata.lowMemMode);

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

	fractParam->doubles.min_y = -10; //-1e10

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
		mainImage.SetPalette(noGUIdata.fractparams.palette);
	}

	InitMainParameters(&fractParam);

	if(netRender->IsServer()) SendSettingsToClients();

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
		autoSaveImage = true;
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
			n = fscanf(pFile_coordinates, "%lf %lf %lf %lf %lf", &fractParam.doubles.vp.x, &fractParam.doubles.vp.y, &fractParam.doubles.vp.z, &fractParam.doubles.alfa,
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
		secondEyeImage->SetPalette(mainImage.GetPalettePtr());
		secondEyeImage->CreatePreview(Interface_data.imageScale);
		secondEyeImage->SetImageParameters(fractParam.doubles.imageAdjustments, fractParam.effectColours, fractParam.imageSwitches);
		stereoImage = new unsigned char[width * height * 3 * 2];
	}

	CVector3 last_vp_position = fractParam.doubles.vp;

	int tiles = fractParam.noOfTiles;

	start_time = real_clock();

	printf("************ Rendering frames *************\n");
	WriteLog("************ Rendering frames *************");
	for (int index = startFrame; index < endFrame; index++)
	{
		WriteLogDouble("Started rendering frame", index);
		//printf("file index %d\n", index);

		//animation with recorded flight path
		double speed = 0.02;

		fractParam.fractal.frameNo = index;

		if (fractParam.animMode && !Interface_data.keyframeMode)
		{
			sprintf(label_text, "Frame: %d", index);
			if (!noGUI)
			{
				gtk_label_set_text(GTK_LABEL(Interface.label_animationFrame), label_text);
			}

			//calculating of mouse pointer position
			int delta_xm, delta_ym;

			delta_xm = x_mouse - width * mainImage.GetPreviewScale() / 2;
			delta_ym = y_mouse - height * mainImage.GetPreviewScale() / 2;

			if (fractParam.recordMode)
			{
				fractParam.doubles.alfa -= 0.0003 * delta_xm;
				fractParam.doubles.beta += 0.0003 * delta_ym;
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
			fractParam.doubles.max_y = 20.0 / fractParam.doubles.zoom;

			//path record mode
			if (fractParam.recordMode && foundLastFrame)
			{

				speed = atof(gtk_entry_get_text(GTK_ENTRY(Interface.edit_animationDESpeed)));

				//calculation of step direction vector
				double delta_x = -sin(fractParam.doubles.alfa) * cos(fractParam.doubles.beta) * distance * speed;
				double delta_y = cos(fractParam.doubles.alfa) * cos(fractParam.doubles.beta) * distance * speed;
				double delta_z = sin(fractParam.doubles.beta) * distance * speed;

				//step
				fractParam.doubles.vp.x += delta_x;
				fractParam.doubles.vp.y += delta_y;
				fractParam.doubles.vp.z += delta_z;

				//saving coordinates to file
				pFile_coordinates = fopen(fractParam.file_path, "a");
				fprintf(pFile_coordinates, "%.10f %.10f %.10f %f %f\n", fractParam.doubles.vp.x, fractParam.doubles.vp.y, fractParam.doubles.vp.z, fractParam.doubles.alfa,
						fractParam.doubles.beta);
				fclose(pFile_coordinates);
				WriteLog("Coordinates saved");
			}

			//path play mode
			if (fractParam.playMode || (fractParam.continueRecord && !foundLastFrame))
			{
				//loading coordinates
				int n;
				n = fscanf(pFile_coordinates, "%lf %lf %lf %lf %lf", &fractParam.doubles.vp.x, &fractParam.doubles.vp.y, &fractParam.doubles.vp.z, &fractParam.doubles.alfa,
						&fractParam.doubles.beta);
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
			fractParam.doubles.max_y = 20.0 / fractParam.doubles.zoom;
			fractParam.doubles.resolution = 1.0 / fractParam.image_width;
			sImageAdjustments imageAdjustments = fractParam.doubles.imageAdjustments;
			mainImage.SetImageAdjustments(imageAdjustments);

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
					printf("alfa = %f, beta = %f\n", fractParam.doubles.alfa, fractParam.doubles.beta);
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
					mRotForEyes.RotateZ(fractParam.doubles.alfa);
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
	fractParamLoaded.doubles.max_y = 20.0 / fractParamLoaded.doubles.zoom;

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
	miniImage->SetImageParameters(fractParamLoaded.doubles.imageAdjustments, fractParamLoaded.effectColours, fractParamLoaded.imageSwitches);
	miniImage->SetPalette(fractParamLoaded.palette);

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
