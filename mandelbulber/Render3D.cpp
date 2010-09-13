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

#include <gtk-2.0/gtk/gtk.h>
#include <math.h>
#include <png.h>
#include <jpeglib.h>
#include <setjmp.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "cimage.hpp"
#include "fractparams.h"
#include "Render3D.h"
#include "algebra.hpp"
#include "interface.h"
#include "image.h"
#include "common_math.h"
#include "files.h"
#include "settings.h"
#include "texture.hpp"
#include "callbacks.h"
#include "shaders.h"
#include "morph.hpp"
#include "undo.hpp"
#include "loadsound.hpp"

bool noGUI = false;

int NR_THREADS;

JSAMPLE *tekstura;
JSAMPLE *lightMap;

int done = 0;

//global conters
guint64 N_counter = 0;
guint64 Loop_counter = 0;
guint64 DE_counter = 0;
guint64 Pixel_counter = 0;
int Missed_DE_counter = 0;
double start_time = 0.0;
bool isRendering = false;
bool isPostRendering = false;

int *thread_done;

cImage *mainImage;

double real_clock(void)
{
#ifdef WIN32 /* WINDOWS */
	return (double) clock() / CLOCKS_PER_SEC;
#else               /*other unix - try sysconf*/
	return (double) clock() / CLOCKS_PER_SEC / NR_THREADS;
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

	sRGB8 black8 = { 0, 0, 0 };
	sRGB16 black16 = { 0, 0, 0 };

	//printf("Thread #%d started\n", z + 1);
	sParamRender param = parametry->param;

	int N = param.N;

	double dist_thresh = param.doubles.dist_thresh;
	double DE_factor = param.doubles.DE_factor;
	CVector3 vp = param.doubles.vp;
	double min_y = param.doubles.min_y;
	double max_y = param.doubles.max_y;
	double resolution = param.doubles.resolution;
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

	sRGB glow_color1 = param.effectColours.glow_color1;
	sRGB glow_color2 = param.effectColours.glow_color2;
	sRGB background_color1 = param.background_color1;
	sRGB background_color2 = param.background_color2;
	bool textured_background = param.textured_background;

	bool fractColor = param.imageSwitches.coloringEnabled;
	cTexture *background_texture = param.backgroundTexture;
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
		}
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

	//distance value after raymarching
	double last_distance = 0;

	//parameters for iteration functions
	sFractal calcParam;

	CopyParams(&param, &calcParam);

	sFractal_ret calcRet;

	double search_accuracy = 0.01;
	double search_limit = 1.0 - search_accuracy;

	WriteLogDouble("All vectors and matrices prepared", thread_number);

	bool sphericalPersp = param.fishEye;
	double fov = param.doubles.persp;

	//2-pass loop (for multi-threading)
	for (int pass = 0; pass < 2; pass++)
	{
		if (pass == 1) start = 0;

		//main loop for z values
		for (z = start; z <= height - progressive; z += progressive)
		{
			//checking if some another thread is not rendering the same z
			if (thread_done[z] == 0)
			{
				//giving information for another threads that this thread renders this z value
				thread_done[z] = start + 1;

				WriteLogDouble("Started rendering line", z);

				//main loop for x values
				for (int x = 0; x <= width - progressive; x += progressive)
				{
					//checking if program was not closed
					if (programClosed)
					{
						return NULL;
					}

					if (progressive < progressiveStart && x % (progressive * 2) == 0 && z % (progressive * 2) == 0) continue;

					//------------- finding fractal surface ---------------------------

					//calculating starting point
					double y_start = max_y;
					if (y_scan_start > min_y) min_y = y_scan_start;
					double x2 = ((double) x / width - 0.5) * zoom * aspectRatio;
					double z2 = ((double) z / height - 0.5) * zoom;

					if (sphericalPersp)
					{
						x2 = M_PI * ((double) x / width - 0.5) * aspectRatio;
						z2 = M_PI * ((double) z / height - 0.5);
					}
					//preparing variables
					bool found = false;
					double dist = delta;
					double stepYpersp = delta;
					counter = 0;
					int binary_step = 0;
					bool binary = false;

					CVector3 viewVectorStart(0, 0, 0);
					CVector3 viewVectorEnd(0, 0, 0);

					if (sphericalPersp)
					{
						min_y = 1e-15;
						max_y = 100;
					}

					//main loop for y values (depth)
					for (double y = min_y; y < max_y; y += stepYpersp)
					{
						//recheck threads
						if (thread_done[z] == start + 1)
						{

							//recalculating normalised y to fractal y
							double y2 = y * zoom;
							//perspective factor (wsp = factor in Polish :-)
							double wsp_persp = 1.0 + y * persp;

							//recalculating dynamic DE threshold
							if (sphericalPersp)
							{
								dist_thresh = 2.0 * y * resolution / quality * fov;
							}
							else
							{
								dist_thresh = zoom * stepY * wsp_persp / quality;
							}

							if (wsp_persp > 0) //checking for sure :-)
							{
								//DE steps counter
								counter++;

								//rotate coordinate system from screen to fractal coordinates
								CVector3 point3D1, point3D2;
								if (sphericalPersp)
								{
									point3D1.x = sin(fov * x2) * y;
									point3D1.z = sin(fov * z2) * y;
									point3D1.y = cos(fov * x2) * cos(fov * z2) * y;
									point3D2 = mRot.RotateVector(point3D1);
								}
								else
								{
									point3D1.x = x2 * wsp_persp;
									point3D1.y = y2;
									point3D1.z = z2 * wsp_persp;
									point3D2 = mRot.RotateVector(point3D1);
								}
								CVector3 point = point3D2 + vp;

								if (counter == 1)
								{
									viewVectorStart = point;
								}

								//calculate opacity

								calcParam.point = point;
								calcParam.DE_threshold = dist_thresh;
								dist = CalculateDistance(calcParam, calcRet);

								//it is for situation when DE is calculated with very big error
								if (dist > 1.0 / DE_factor)
								{
									dist = 1.0 / DE_factor;
								}

								//if maxiter then distance is zero
								if (calcRet.max_iter)
								{
									dist = 0;
								}

								//iteration threshold mode
								if (param.iterThresh)
								{
									if (dist < dist_thresh && !calcRet.max_iter)
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
									Missed_DE_counter++;
									break;
								}

								//step in DE mode
								if (!binary)
								{
									if (sphericalPersp) stepYpersp = dist * DE_factor;
									else stepYpersp = dist / zoom * DE_factor; //0.75
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
									if (binary_step > 10 || (dist < dist_thresh && dist > dist_thresh * search_limit))
									{
										found = true;
										y_start = y;

										last_distance = dist;

										break;
									}
								}
							}
						}
					} //next y
					DE_counter += counter;
					Pixel_counter++;
					//counters for drawing histogram
					int counter2 = counter / 4;
					if (counter2 < 256) histogram2[counter2]++;
					else histogram2[255]++;

					//if fractal surface was found
					if (found)
					{
						//-------------------- SHADING ----------------------
						double y = y_start;

						//translate coordinate system from screen to fractal
						double y2 = y * zoom;
						double wsp_persp = 1.0 + y * persp;
						CVector3 point3D1, point3D2;
						if (sphericalPersp)
						{
							point3D1.x = sin(fov * x2) * y;
							point3D1.z = sin(fov * z2) * y;
							point3D1.y = cos(fov * x2) * cos(fov * z2) * y;
							point3D2 = mRot.RotateVector(point3D1);
						}
						else
						{
							point3D1.x = x2 * wsp_persp;
							point3D1.y = y2;
							point3D1.z = z2 * wsp_persp;
							point3D2 = mRot.RotateVector(point3D1);
						}
						CVector3 point = point3D2 + vp;

						viewVectorEnd = point;

						CVector3 viewVector = viewVectorEnd - viewVectorStart;
						viewVector.Normalize();

						CVector3 lightVector = vDelta;
						//sVector lightVector = { -0.702, -0.702, -0.702 };

						//normal vector
						CVector3 vn = CalculateNormals(&param, &calcParam, point, wsp_persp, dist_thresh, last_distance);

						//delta for all shading algorithms depended on depth and resolution (dynamic shaders resolution)
						if (sphericalPersp)
						{
							delta = resolution * y * fov;
							wsp_persp = 2.0 * y;
							zoom = param.doubles.zoom = 1.0;
						}
						else
						{
							delta = resolution * zoom * wsp_persp;
						}

						//********* calculate hard shadow
						unsigned short shadow16 = 4096;
						if (shadow)
						{
							sShaderOutput shadowOutput = MainShadow(&param, &calcParam, point, lightVector, wsp_persp, dist_thresh);
							shadow16 = shadowOutput.R * 4096.0;
						}

						image->PutPixelShadow(x, z, shadow16);

						//******* calculate global illumination (ambient occlusion)

						sShaderOutput AO = { 0, 0, 0 };

						if (global_ilumination)
						{
							//ambient occlusion based on orbit traps
							if (fastGlobalIllumination)
							{
								calcParam.point = point;
								calcParam.mode = fake_AO;
								ComputeIterations(calcParam, calcRet);
								double j = (calcRet.fake_ao - 0.65) * 1.0; //0.65
								if (j > 1.0) j = 1.0;
								if (j < 0) j = 0;
								AO.R = j;
								AO.G = j;
								AO.B = j;
							}

							//ambient occlusion based on many rays in spherical directions

							else
							{
								AO = AmbientOcclusion(&param, &calcParam, point, wsp_persp, dist_thresh, last_distance, vectorsAround, vectorsCount, vn);
							}
							sRGB16 globalLight = { AO.R * 4096.0, AO.G * 4096.0, AO.B * 4096.0 };
							image->PutPixelAmbient(x, z, globalLight);
						}

						//calculate shading based on angle of incidence
						sShaderOutput shade = MainShading(vn, lightVector);
						image->PutPixelShading(x, z, shade.R * 4096.0);

						int numberOfLights = lightsPlaced;
						if (numberOfLights < 4) numberOfLights = 4;
						bool accurate;
						sShaderOutput shadeAuxSum = { 0, 0, 0 };
						sShaderOutput specularAuxSum = { 0, 0, 0 };
						for (int i = 0; i < numberOfLights; i++)
						{
							if (i < param.auxLightNumber || Lights[i].enabled)
							{
								if (i < 4) accurate = true;
								else accurate = false;

								sShaderOutput specularAuxOut;
								sShaderOutput shadeAux = LightShading(&param, &calcParam, point, vn, viewVector, Lights[i], wsp_persp, dist_thresh, numberOfLights, &specularAuxOut, accurate);
								shadeAuxSum.R += shadeAux.R;
								shadeAuxSum.G += shadeAux.G;
								shadeAuxSum.B += shadeAux.B;
								specularAuxSum.R += specularAuxOut.R;
								specularAuxSum.G += specularAuxOut.G;
								specularAuxSum.B += specularAuxOut.B;
							}
						}
						sRGB16 shadeAux16 = { shadeAuxSum.R * 4096.0, shadeAuxSum.G * 4096.0, shadeAuxSum.B * 4096.0 };
						image->PutPixelAuxLight(x, z, shadeAux16);

						sRGB16 specularAux16 = { specularAuxSum.R * 4096.0, specularAuxSum.G * 4096.0, specularAuxSum.B * 4096.0 };
						image->PutPixelAuxSpecular(x, z, specularAux16);

						//calculate specular reflection effect
						sShaderOutput specular = MainSpecular(vn, lightVector, viewVector);
						image->PutPixelSpecular(x, z, specular.R * 4096.0);

						//calculate environment mapping reflection effect
						sShaderOutput envMapping = EnvMapping(vn, viewVector, envmap_texture);
						sRGB8 reflection = { envMapping.R * 256.0, envMapping.G * 256.0, envMapping.B * 256.0 };
						image->PutPixelReflect(x, z, reflection);

						//coloured surface of fractal
						int colorIndex = 0;
						if (fractColor)
						{
							{
								calcParam.point = point;
								calcParam.mode = colouring;
								calcParam.N = N * 10;
								ComputeIterations(calcParam, calcRet);
								calcParam.N = N;
								int nrKol = calcRet.colour;
								nrKol = abs(nrKol) % 65536;
								colorIndex = nrKol;
							}
						}
						image->PutPixelColor(x, z, colorIndex);

						//zBuffer
						image->PutPixelZBuffer(x, z, y);
						image->PutPixelBackground(x, z, black16);

					}//end if found

					else
					{
						image->PutPixelAmbient(x, z, black16);
						image->PutPixelAuxLight(x, z, black16);
						image->PutPixelAuxSpecular(x, z, black16);
						image->PutPixelColor(x, z, 0);
						image->PutPixelReflect(x, z, black8);
						image->PutPixelShading(x, z, 0);
						image->PutPixelShadow(x, z, 0);
						image->PutPixelSpecular(x, z, 0);

						//------------- render 3D background
						if (textured_background)
						{
							double y = 1e20;

							//translate coordinate system from screen to fractal
							double y2 = y * zoom;
							double wsp_persp = 1.0 + y * persp;
							CVector3 point3D1, point3D2;

							if (sphericalPersp)
							{
								point3D1.x = sin(fov * x2) * y;
								point3D1.z = sin(fov * z2) * y;
								point3D1.y = cos(fov * x2) * cos(fov * z2) * y;
								point3D2 = mRot.RotateVector(point3D1);
							}
							else
							{
								point3D1.x = x2 * wsp_persp;
								point3D1.y = y2;
								point3D1.z = z2 * wsp_persp;
								point3D2 = mRot.RotateVector(point3D1);
							}
							CVector3 point = point3D2 + vp;

							//calculate texture coordinates
							double alfaTexture = point.GetAlfa() + M_PI;
							double betaTexture = point.GetBeta();

							if (betaTexture > 0.5 * M_PI) betaTexture = 0.5 * M_PI - betaTexture;

							if (betaTexture < -0.5 * M_PI) betaTexture = -0.5 * M_PI + betaTexture;

							double texX = alfaTexture / (2.0 * M_PI) * background_texture->Width();
							double texY = (betaTexture / (M_PI) + 0.5) * background_texture->Height();

							sRGB8 pixel = background_texture->Pixel(texX, texY);
							sRGB16 pixel16;
							pixel16.R = pixel.R * 256;
							pixel16.G = pixel.G * 256;
							pixel16.B = pixel.B * 256;
							image->PutPixelBackground(x, z, pixel16);
							//PutPixelAlfa(x, z, pixel.R * 256, pixel.G * 256, pixel.B * 256, 65536, 1.0);
						}
						else
						{
							sRGB16 pixel;
							double grad = ((double) x + z) / (width + height);
							double Ngrad = 1.0 - grad;
							pixel.R = (background_color1.R * Ngrad + background_color2.R * grad);
							pixel.G = (background_color1.G * Ngrad + background_color2.G * grad);
							pixel.B = (background_color1.B * Ngrad + background_color2.B * grad);
							image->PutPixelBackground(x, z, pixel);
							//PutPixelAlfa(x, z, backR, backG, backB, 65536, 1.0);
						}
						image->PutPixelZBuffer(x, z, 1e20);
					}

					//drawing transparent glow
					image->PutPixelGlow(x, z, counter);

				}//next x

				image->Squares(z, progressive);

				done++;

				double progressiveDone;
				if (progressive == progressiveStart) progressiveDone = 0;
				else progressiveDone = 0.25 / (progressive * progressive);
				double percent_done = (((double) done / height) * 3.0 / 4.0 / progressive + progressiveDone) * 100.0;
				double time = real_clock() - start_time;
				double time_to_go = (100.0 - percent_done) * time / percent_done;
				int togo_time_s = (int) time_to_go % 60;
				int togo_time_min = (int) (time_to_go / 60) % 60;
				int togo_time_h = time_to_go / 3600;

				int time_s = (int) time % 60;
				int time_min = (int) (time / 60) % 60;
				int time_h = time / 3600;

				double iterations_per_sec = N_counter / time;
				printf("Done %.3f%%, to go = %dh%dm%ds, elapsed = %dh%dm%ds, iter/s = %.0f       \r", percent_done, togo_time_h, togo_time_min, togo_time_s, time_h, time_min, time_s,
						iterations_per_sec);
				fflush(stdout);

				//printing to console some statistics
				if (done == height - 1)
				{
					double avg_N = (double) N_counter / Loop_counter;
					double avg_DE = (double) DE_counter / Pixel_counter;
					printf("Average N = %f, Average DE steps = %f, Faied DE = %d\n", avg_N, avg_DE, Missed_DE_counter);
				}

				WriteLogDouble("Rendering line finished", z);
			}//end if thread done

		}//next z
	}// next pass
	WriteLogDouble("Thread finished", thread_number);
	delete[] vectorsAround;
	return 0;

}

//************************** Render **************************************
void Render(sParamRender param, cImage *image, GtkWidget *outputDarea)
{
	Interface_data.disableInitRefresh = false;
	programClosed = false;
	//getting image resolution from parameters
	int width = image->GetWidth();
	int height = image->GetHeight();

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
	start_time = real_clock();

	//initialising threads
	thread_done = new int[height];
	GThread *Thread[NR_THREADS + 1];
	GError *err[NR_THREADS + 1];
	sParam thread_param[NR_THREADS];

	int progressiveStart = 16;
	image->progressiveFactor = progressiveStart;

	int refresh_index = 0;
	int refresh_skip = 1;

	for (int progressive = progressiveStart; progressive != 0; progressive /= 2)
	{
		//if (width * height / progressive / progressive <= 100 * 100) bo_refreshing = false;
		//else bo_refreshing = true;

		done = 0;
		for (int i = 0; i < NR_THREADS; i++)
		{
			err[i] = NULL;
		}
		for (int i = 0; i < height; i++)
		{
			thread_done[i] = 0;
		}
		WriteLog("Threads data prepared");

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
			//creating thread
			Thread[i] = g_thread_create((GThreadFunc) MainThread, &thread_param[i], TRUE, &err[i]);
			WriteLogDouble("Thread created", i);
			//printf("Rendering thread #%d created\n", i + 1);
		}

		//refresh GUI
		if (!noGUI && bo_refreshing)
		{
			while (gtk_events_pending())
				gtk_main_iteration();

			//refreshing image and histograms during rendering

			while (done < height / progressive - 1 && !programClosed)
			{
				g_usleep(100000);
				if (progressive < progressiveStart)
				{
					refresh_index++;
					//if (image->IsPreview()) mainImage->RedrawInWidget(darea);

					if (refresh_index >= refresh_skip)
					{
						double refreshStartTime = real_clock();

						if (Interface_data.SSAOEnabled)
						{
							PostRendering_SSAO(image, param.doubles.persp, Interface_data.SSAOQuality / 2);
							WriteLog("SSAO rendered");
						}
						image->CompileImage();
						WriteLog("Image compiled");

						if (image->IsPreview())
						{
							image->ConvertTo8bit();
							WriteLog("Image converted into 8-bit");
							image->UpdatePreview();
							WriteLog("Preview created");
							if (outputDarea != NULL)
							{
								DrawHistogram();
								DrawHistogram2();
								WriteLog("Histograms refreshed");
								image->RedrawInWidget(outputDarea);
								WriteLog("Image redrawed in window");
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
					double progressiveDone;
					if (progressive == progressiveStart) progressiveDone = 0;
					else progressiveDone = 0.25 / (progressive * progressive);
					double percent_done = (((double) done / height) * 3.0 / 4.0 / progressive + progressiveDone) * 100.0;
					double time = real_clock() - start_time;
					double time_to_go = (100.0 - percent_done) * time / percent_done;
					int togo_time_s = (int) time_to_go % 60;
					int togo_time_min = (int) (time_to_go / 60) % 60;
					int togo_time_h = time_to_go / 3600;
					int time_s = (int) time % 60;
					int time_min = (int) (time / 60) % 60;
					int time_h = time / 3600;
					double iterations_per_sec = N_counter / time;
					sprintf(progressText, "%.3f%%, to go %dh%dm%ds, elapsed %dh%dm%ds, iters/s %.0f", percent_done, togo_time_h, togo_time_min, togo_time_s, time_h, time_min, time_s,
							iterations_per_sec);
					gtk_progress_bar_set_text(GTK_PROGRESS_BAR(Interface.progressBar), progressText);
					gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(Interface.progressBar), percent_done / 100.0);
				}
				//refreshing GUI
				while (gtk_events_pending())
					gtk_main_iteration();
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
	}//next progress
	printf("\n");

	//refreshing image

	//*** postprocessing
	//guint *post_bitmap = new guint[IMAGE_WIDTH * IMAGE_HEIGHT * 3];
	//PostRendering_SSAO(rgbbuf32, post_bitmap, zBuffer, param.persp, param.globalIlum);
	//Bitmap32to8(post_bitmap, rgbbuf, rgbbuf2);
	//delete post_bitmap;
	//end of postprocessing
	if (Interface_data.SSAOEnabled && !programClosed)
	{
		PostRendering_SSAO(image, param.doubles.persp, Interface_data.SSAOQuality);
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
			WriteLog("Image converted into 8-bit");
			image->UpdatePreview();
			WriteLog("Preview created");
			if (outputDarea != NULL)
			{
				DrawHistogram();
				DrawHistogram2();
				WriteLog("Histograms refreshed");
				image->RedrawInWidget(outputDarea);
				WriteLog("Image redrawed in window");
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
			sprintf(progressText, "Rendering done, elapsed %dh%dm%ds, iters/s %.0f, average N %.1f, average DE steps %.1f", time_h, time_min, time_s, iterations_per_sec, avg_N, avg_DE);
			gtk_progress_bar_set_text(GTK_PROGRESS_BAR(Interface.progressBar), progressText);
			gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(Interface.progressBar), 1.0);
		}
		while (gtk_events_pending())
			gtk_main_iteration();
	}
	delete[] thread_done;
}

//******************************** Get number of CPU cores *************

int get_cpu_count()
{
	int ret;

#ifdef WIN32 /* WINDOWS */
	SYSTEM_INFO info;

	GetSystemInfo(&info);
	ret = info.dwNumberOfProcessors;
#else               /*other unix - try sysconf*/
	ret = (int) sysconf(_SC_NPROCESSORS_ONLN);
#endif  /* WINDOWS */
	return ret;
}

//********************************** MAIN ******************************* 
int main(int argc, char *argv[])
{
	//read $home env variable
	char *homedir;
	char data_directory[1000];

#ifdef WIN32 /* WINDOWS */
	homedir = getenv("USERPROFILE");
#else               /*other unix - try sysconf*/
	homedir = getenv("HOME");
#endif  /* WINDOWS */

	//logfile
#ifdef WIN32 /* WINDOWS */
	sprintf(logfileName, "log.txt");
#else
	sprintf(logfileName, "%s/.mandelbulber_log.txt", homedir);
#endif
	FILE *logfile = fopen(logfileName, "w");
	fclose(logfile);

	printf("Log file: %s\n", logfileName);
	WriteLog("Log file created");

	//initialising GTK+
	bool result = ReadComandlineParams(argc, argv);
	WriteLogDouble("Parameters got from console", argc);
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
	printf("Detected %d CPUs\n", NR_THREADS);
	WriteLogDouble("CPUs detected", NR_THREADS);

	//lokckout for refreshing image during program startup
	Interface_data.disableInitRefresh = true;

	int width = 800;
	int height = 600;

	mainImage = new cImage(width, height);
	WriteLog("complexImage allocated");

	//allocating memory for image in window
	mainImage->CreatePreview(1.0);
	WriteLog("Memory allocated for preview");

	//allocating memory for lights
	Lights = new sLight[10000];
	WriteLog("memory for lights allocated");
	printf("Memory allocated\n");

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

	sParamRender fractParamDefault;
	ParamsAllocMem(&fractParamDefault);
	WriteLog("allocated memory for default parameters");

	bool noGUIsettingsLoaded = false;
	if (noGUI)
	{
		ParamsAllocMem(&noGUIdata.fractparams);
		if (LoadSettings(noGUIdata.settingsFile, noGUIdata.fractparams))
		{
			noGUIsettingsLoaded = true;
			WriteLog("Parameters loaded in noGUI mode");
		}
	}

	sprintf(data_directory, "%s/.mandelbulber", homedir);
	printf("Default data directory: %s\n", data_directory);

	strcpy(lastFilenameSettings, "settings/default.fract");
	strcpy(lastFilenameImage, "images/image.jpg");
	strcpy(lastFilenamePalette, "textures/palette.jpg");

	//set program home directory
	if (!chdir(data_directory))
	{
		//loading undo buffer status
		undoBuffer.LoadStatus();
		WriteLog("Undo buffer status loaded");

		//reading default configuration in GUI mode
		if (!noGUI)
		{
			if (LoadSettings((char*) "settings/default.fract", fractParamDefault))
			{
				WriteLog("Default settings loaded");
				printf("Default settings loaded succesfully (settings/default.fract)\n");
				//creating GTK+ GUI
				CreateInterface(&fractParamDefault);
			}
			else
			{
				printf("Can't open default settings file: %s/settings/default.fract\n", data_directory);
				WriteLog("Can't open default settings file");
				WriteLog(data_directory);
			}
		}
		else
		{
			if (!noGUIsettingsLoaded)
			{
				char settingsFile2[1000];
				sprintf(settingsFile2, "settings/%s", noGUIdata.settingsFile);
				if (!LoadSettings((char*) settingsFile2, noGUIdata.fractparams))
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

		ParamsReleaseMem(&fractParamDefault);
		WriteLog("Memory for parameters released");
	}
	else
	{
		printf("Can't acces default data directory: %s\n", data_directory);
		WriteLog("Can't acces default data directory");
		WriteLog(data_directory);
	}
	delete mainImage;
	return (0);
}

//****************************8 MAIN called by "Render" button
void MainRender(void)
{
	isRendering = true;

	//allocating memory for fractal parameters
	sParamRender fractParam;
	sParamSpecial fractSpecial;
	ParamsAllocMem(&fractParam);
	WriteLog("Memory allocated for fractal parameters");

	//reading parameters from interface
	if (!noGUI)
	{
		ReadInterface(&fractParam, &fractSpecial);
		printf("Data has been read from interface\n");
		WriteLog("Data got from interface");
	}
	else
	{
		memcpy(&fractParam, &noGUIdata.fractparams, sizeof(sParamRender));
		ReadInterface(&fractParam, &fractSpecial);
		Interface_data.imageFormat = noGUIdata.imageFormat;
		WriteLog("Data got from interface");
	}

	fractParam.doubles.min_y = -10; //-1e10

	//animation/render mode
	fractParam.playMode = Interface_data.playMode;
	fractParam.animMode = Interface_data.animMode;
	fractParam.recordMode = Interface_data.recordMode;
	fractParam.continueRecord = Interface_data.continueRecord;

	//image size
	int width = fractParam.image_width;
	int height = fractParam.image_height;

	mainImage->ChangeSize(width, height);
	WriteLog("complexImage allocated");
	printf("Memory for image: %d MB\n", mainImage->GetUsedMB());

	if (noGUI)
	{
		mainImage->SetPalette(noGUIdata.fractparams.palette);
	}

	mainImage->CreatePreview(Interface_data.imageScale);

	if (!noGUI)
	{
		gtk_widget_set_size_request(darea, mainImage->GetPreviewWidth(), mainImage->GetPreviewHeight());
	}

	WriteLog("rgbbuf allocated");

	//waiting for refresh window
	g_usleep(100000);
	if (!noGUI)
	{
		for (int i = 0; i < 10; i++)
		{
			while (gtk_events_pending())
				gtk_main_iteration();
		}
	}
	WriteLog("Windows refreshed");

	printf("Memory reallocated\n");

	//loading texture for environment mapping
	fractParam.envmapTexture = new cTexture(fractParam.file_envmap);
	if (fractParam.envmapTexture->IsLoaded())
	{
		printf("Environment map texture loaded\n");
		WriteLog("Environment map texture loaded");
	}
	else
	{
		printf("Error! Can't open envmap texture: %s\n", fractParam.file_envmap);
		WriteLog("Error! Can't open envmap texture");
		WriteLog(fractParam.file_envmap);
		if (!noGUI)
		{
			GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window_interface), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CANCEL, "Error loading envmap texture file: %s",
					fractParam.file_envmap);
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);
		}
		isRendering = false;
		return;
	}

	//loading texture for ambient occlusion light map
	fractParam.lightmapTexture = new cTexture(fractParam.file_lightmap);
	if (fractParam.lightmapTexture->IsLoaded())
	{
		printf("Ambient occlusion light map texture loaded\n");
		WriteLog("Ambient occlusion map texture loaded");
	}
	else
	{
		printf("Error! Can't open ambient occlusion light map texture:%s\n", fractParam.file_lightmap);
		WriteLog("Error! Can't open ambient occlusion light map texture");
		WriteLog(fractParam.file_lightmap);
		if (!noGUI)
		{
			GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window_interface), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CANCEL,
					"Error! Can't open ambient occlusion light map texture:\n%s", fractParam.file_lightmap);
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);
		}
		isRendering = false;
		return;
	}

	//generating color palette
	//srand(fractParam.coloring_seed);
	//NowaPaleta(paleta, 1.0);
	WriteLog("New colour palette created");
	if (!noGUI)
	{
		DrawPalette(fractParam.palette);
		WriteLog("Pallete refreshed on GUI");
	}
	//placing random Lights
	if (lightsPlaced == 0)
	{
		PlaceRandomLights(&fractParam);
		WriteLog("Lights placed");
	}
	printf("Lights placed\n");

	//reading background texture
	cTexture *backgroundTexture = new cTexture(fractParam.file_background);
	fractParam.backgroundTexture = backgroundTexture;
	if (fractParam.backgroundTexture->IsLoaded())
	{
		printf("Background texture loaded\n");
		WriteLog("Background texture loaded");
	}
	else
	{
		printf("Error! Can't open background texture:%s\n", fractParam.file_background);
		WriteLog("Error! Can't open background texture");
		WriteLog(fractParam.file_background);
		if (!noGUI)
		{
			GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window_interface), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CANCEL,
					"Error! Can't open background texture:\n%s", fractParam.file_background);
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);
		}
		isRendering = false;
		return;
	}

	//loading sound
	if (fractParam.soundEnabled)
	{
		sound.Load((char*) fractParam.file_sound);
		WriteLog("Sound file loaded");
		sound.SetFPS(25.0);
		sound.CreateEnvelope();
		WriteLog("Sound envelope calculated");
		int bandMin[4] = { fractParam.soundBand1Min, fractParam.soundBand2Min, fractParam.soundBand3Min, fractParam.soundBand4Min };
		int bandMax[4] = { fractParam.soundBand1Max, fractParam.soundBand2Max, fractParam.soundBand3Max, fractParam.soundBand4Max };
		sound.DoFFT(bandMin, bandMax);
		WriteLog("Sound FFT calculated");
	}

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
	char filename2[1000];
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
			IndexFilename(filename2, fractParam.file_keyframes, (char*) "fract", maxKeyNumber);
			maxKeyNumber++;
		} while (FileIfExist(filename2));
		WriteLog("Keyframes counted");
	}
	else
	{
		maxKeyNumber = 1;
	}
	maxKeyNumber--;
	if (Interface_data.keyframeMode) printf("Found %d keyframes\n", maxKeyNumber);

	//loading keyframes in keyframe animation mode
	CMorph morph(maxKeyNumber, sizeof(sParamRenderD) / sizeof(double));
	CMorph morphIFS(maxKeyNumber, 8 * IFS_number_of_vectors);
	double *IFSdouble = new double[8 * IFS_number_of_vectors];
	WriteLog("Memory for morphing allocated");

	morph.SetFramesPerKey(fractParam.framesPerKeyframe);
	morphIFS.SetFramesPerKey(fractParam.framesPerKeyframe);

	if (Interface_data.keyframeMode)
	{
		for (int keyNumber = 0; keyNumber < maxKeyNumber; keyNumber++)
		{
			IndexFilename(filename2, fractParam.file_keyframes, (char*) "fract", keyNumber);

			sParamRender fractParamLoaded;
			ParamsAllocMem(&fractParamLoaded);
			LoadSettings(filename2, fractParamLoaded);
			WriteLogDouble("Keyframe loaded", keyNumber);
			morph.AddData(keyNumber, (double*) &fractParamLoaded);
			IFSToMorph(IFSdouble, &fractParamLoaded);
			morphIFS.AddData(keyNumber, IFSdouble);
			WriteLogDouble("Keyframe data added to data structures", keyNumber);
			ParamsReleaseMem(&fractParamLoaded);
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

	if (Interface_data.keyframeMode)
	{
		startFrame = fractParam.startFrame;
		endFrame = fractParam.endFrame;
		if (noGUI && noGUIdata.startFrame > 0) startFrame = noGUIdata.startFrame;
		if (noGUI && noGUIdata.endFrame < 99999) endFrame = noGUIdata.endFrame;
	}

	cImage *secondEyeImage;
	unsigned char *stereoImage = 0;
	if (fractParam.stereoEnabled)
	{
		autoSaveImage = true;
		secondEyeImage = new cImage(width, height);
		secondEyeImage->SetPalette(mainImage->GetPalettePtr());
		secondEyeImage->CreatePreview(Interface_data.imageScale);
		secondEyeImage->SetImageParameters(fractParam.doubles.imageAdjustments,fractParam.effectColours,fractParam.imageSwitches);
		stereoImage = new unsigned char[width * height * 3 * 2];
	}

	CVector3 last_vp_position = fractParam.doubles.vp;

	printf("************ Rendering frames *************\n");
	WriteLog("************ Rendering frames *************");
	for (int index = startFrame; index < endFrame; index++)
	{
		WriteLogDouble("Started rendering frame", index);
		//printf("file index %d\n", index);

		//animation with recorded flight path
		double speed = 0.02;
		if (fractParam.animMode && !Interface_data.keyframeMode)
		{
			sprintf(label_text, "Frame: %d", index);
			if (!noGUI)
			{
				gtk_label_set_text(GTK_LABEL(Interface.label_animationFrame), label_text);
			}
			//calculating of mouse pointer position
			int delta_xm, delta_ym;

			delta_xm = x_mouse - width * mainImage->GetPreviewScale() / 2;
			delta_ym = y_mouse - height * mainImage->GetPreviewScale() / 2;

			if (fractParam.recordMode)
			{
				fractParam.doubles.alfa -= 0.0003 * delta_xm;
				fractParam.doubles.beta += 0.0003 * delta_ym;
			}

			//calculation of distance to fractal surface
			sFractal calcParam;
			sFractal_ret calcRet;
			CopyParams(&fractParam, &calcParam);
			calcParam.point = fractParam.doubles.vp;
			distance = CalculateDistance(calcParam, calcRet);
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
				WriteLog("Coordinate saved");
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
				WriteLog("Coordinate loaded");
			}
		}

		//Catmull-Rom interpolation in keyframe animation mode
		if (fractParam.animMode && Interface_data.keyframeMode)
		{
			morph.CatmullRom(index, (double*) &fractParam);
			morphIFS.CatmullRom(index, IFSdouble);
			WriteLog("Splines calculated");
			MorphToIFS(IFSdouble, &fractParam);
			if (fractParam.doubles.zoom < 1e-15) fractParam.doubles.zoom = 1e-15;
			fractParam.doubles.max_y = 20.0 / fractParam.doubles.zoom;
			fractParam.doubles.resolution = 1.0 / fractParam.image_width;
			sImageAdjustments imageAdjustments = fractParam.doubles.imageAdjustments;
			mainImage->SetImageAdjustments(imageAdjustments);
			Interface_data.persp = fractParam.doubles.persp;

			sprintf(label_text, "Frame: %d, Keyframe %f", index, (double) index / fractParam.framesPerKeyframe);
			if (!noGUI) gtk_label_set_text(GTK_LABEL(Interface.label_keyframeInfo), label_text);
			WriteLog("Calculated additional data for splines");
		}

		//animation by sound
		if (fractParam.soundEnabled)
		{
			sAddData *addData;
			double *paramToAdd;
			addData = (sAddData*) &fractSpecial;
			paramToAdd = (double*) &fractParam;
			for (unsigned int i = 0; i < sizeof(sParamRenderD); i++)
			{
				if (addData->mode == soundEnvelope) *paramToAdd += sound.GetEnvelope(index) * addData->amp;
				else if (addData->mode == soundA) *paramToAdd += sound.GetSpectrumA(index) * addData->amp;
				else if (addData->mode == soundB) *paramToAdd += sound.GetSpectrumB(index) * addData->amp;
				else if (addData->mode == soundC) *paramToAdd += sound.GetSpectrumC(index) * addData->amp;
				else if (addData->mode == soundD) *paramToAdd += sound.GetSpectrumD(index) * addData->amp;

				fractParam.doubles.max_y = 20.0 / fractParam.doubles.zoom;
				fractParam.doubles.resolution = 1.0 / fractParam.image_width;
				sImageAdjustments imageAdjustments = fractParam.doubles.imageAdjustments;
				mainImage->SetImageAdjustments(imageAdjustments);
				Interface_data.persp = fractParam.doubles.persp;

				addData++;
				paramToAdd++;
			}
			WriteLog("Sound animation calculated");
		}
		RecalculateIFSParams(&fractParam);
		WriteLog("IFS params recalculated");

		CreateFormulaSequence(&fractParam);
		WriteLog("Formula sequence created");

		if (!noGUI)
		{
			if (fractParam.animMode) WriteInterface(&fractParam);
			else WriteInterface(&fractParam, &fractSpecial);
			WriteLog("GUI data refreshed");
		}

		if (Interface_data.imageFormat == imgFormatJPG)
		{
			IndexFilename(filename2, fractParam.file_destination, (char*) "jpg", index);
		}
		else
		{
			IndexFilename(filename2, fractParam.file_destination, (char*) "png", index);
		}
		FILE *testFile;
		testFile = fopen(filename2, "r");
		if ((testFile != NULL && !fractParam.recordMode && (autoSaveImage || fractParam.animMode)) || (!foundLastFrame && fractParam.continueRecord))
		{
			if (!fractParam.animMode)
			{
				//printf("Output file exists. Skip to next file index\n");
			}
		}
		else
		{

			//renderowanie fraktala
			//if ((index % frame_step == 0) || record_mode)

			if (fractParam.animMode)
			{
				sFractal calcParam;
				sFractal_ret calcRet;
				CopyParams(&fractParam, &calcParam);
				distance = CalculateDistance(calcParam, calcRet);
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

				//clear image


				if (eye == 0)
				{
					mainImage->ClearImage();
					WriteLog("Image cleared");
					Render(fractParam, mainImage, darea);
					WriteLog("Image rendered");
				}
				else if (eye == 1)
				{
					secondEyeImage->ClearImage();
					WriteLog("Image cleared");
					Render(fractParam, secondEyeImage, darea);
					WriteLog("Image rendered");
					MakeStereoImage(mainImage, secondEyeImage, stereoImage);
					WriteLog("Stereo image maked");
					StereoPreview(mainImage,stereoImage);
					IndexFilename(filename2, fractParam.file_destination, (char*) "jpg", index);
					SaveJPEG(filename2, 100, width * 2, height, (JSAMPLE*) stereoImage);
					WriteLog("Stereo image saved");
					if (!noGUI)
					{
						char progressText[1000];
						sprintf(progressText, "Stereoscopic image was saved to: %s", filename2);
						gtk_progress_bar_set_text(GTK_PROGRESS_BAR(Interface.progressBar), progressText);
					}
				}

				//save image
				if ((autoSaveImage || fractParam.animMode) && !fractParam.stereoEnabled)
				{
					unsigned char *rgbbuf2 = mainImage->ConvertTo8bit();
					if (Interface_data.imageFormat == imgFormatJPG)
					{
						IndexFilename(filename2, fractParam.file_destination, (char*) "jpg", index);
						SaveJPEG(filename2, 100, width, height, (JSAMPLE*) rgbbuf2);
					}
					else if (Interface_data.imageFormat == imgFormatPNG)
					{
						IndexFilename(filename2, fractParam.file_destination, (char*) "png", index);
						SavePNG(filename2, 100, width, height, (png_byte*) rgbbuf2);
					}
					else if (Interface_data.imageFormat == imgFormatPNG16)
					{
						IndexFilename(filename2, fractParam.file_destination, (char*) "png", index);
						SavePNG16(filename2, 100, width, height, (png_byte*) rgbbuf2);
					}
					else if (Interface_data.imageFormat == imgFormatPNG16Alpha)
					{
						IndexFilename(filename2, fractParam.file_destination, (char*) "png", index);
						SavePNG16Alpha(filename2, 100, width, height, (png_byte*) rgbbuf2);
					}
					printf("Image saved: %s\n", filename2);
					WriteLog("Image saved");
					WriteLog(filename2);
				}

			}
			//terminate animation loop when not animation mode
			if (!fractParam.animMode)
			{
				index = 9999999;
			}

			last_vp_position = fractParam.doubles.vp;

			if (programClosed) break;
		}

		if (testFile != NULL)
		{
			fclose(testFile);
		}

	}

	if (fractParam.playMode && !Interface_data.keyframeMode) fclose(pFile_coordinates);

	delete fractParam.backgroundTexture;
	WriteLog("Released memory for background texture");
	delete fractParam.envmapTexture;
	WriteLog("Released memory for envmap texture");
	delete fractParam.lightmapTexture;
	WriteLog("Released memory for lightmap texture");
	delete[] IFSdouble;
	WriteLog("Released memory for IFS params");
	ParamsReleaseMem(&fractParam);
	WriteLog("Released memory for fractal params");

	if (fractParam.stereoEnabled)
	{
		delete[] secondEyeImage;
		delete[] stereoImage;
		WriteLog("Released memory for stereo image");
	}
	printf("Rendering finished\n");
	WriteLog("Rendering completely finished");
	isRendering = false;
}

