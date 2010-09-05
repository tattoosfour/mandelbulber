/*
 * settings.h
 *
 *  Created on: 2010-01-24
 *      Author: krzysztof marczak
 */

#ifndef SETTINGS_H_
#define SETTINGS_H_

#include "Render3D.h"

void fprintfDot(FILE *file, const char *string, double value, sAddData *addData = 0);
double atof2(char *str, bool locale_dot, sAddData *addData = 0);
void SaveSettings(char *filename, sParamRender params, sParamSpecial *special = 0);
bool LoadSettings(char *filename, sParamRender &params, sParamSpecial *special = 0);
void DefaultValues(sParamRender *params);
void ParamsAllocMem(sParamRender *fractParam);
void ParamsReleaseMem(sParamRender *fractParam);
void IFSToMorph(double *IFSdouble, sParamRender *params);
void MorphToIFS(double *IFSdouble, sParamRender *params);
void MakePaletteString(sRGB *palette, char *paletteString);
void GetPaletteFromString(sRGB *palette, char *paletteString);
#endif /* SETTINGS_H_ */
