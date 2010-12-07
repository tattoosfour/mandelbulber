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
void SaveSettings(char *filename, const sParamRender &params, sParamSpecial *special = 0);
bool LoadSettings(char *filename, sParamRender &params, sParamSpecial *special = 0, bool disableMessages = false);
void DefaultValues(sParamRender &params);
void IFSToMorph(double *IFSdouble, const sFractal &fractal);
void MorphToIFS(double *IFSdouble, sFractal &fractal);
void MakePaletteString(const sRGB *palette, char *paletteString);
void GetPaletteFromString(sRGB *palette, char *paletteString);
void KeepOtherSettings(sParamRender *params);
#endif /* SETTINGS_H_ */
