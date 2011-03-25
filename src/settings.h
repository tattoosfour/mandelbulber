/*
 * settings.h
 *
 *  Created on: 2010-01-24
 *      Author: krzysztof marczak
 */

#ifndef SETTINGS_H_
#define SETTINGS_H_

#include "Render3D.h"

void fprintfDot(FILE *file, const char *string, double value);
double atof2(char *str, bool locale_dot);
void SaveSettings(char *filename, const sParamRender &params);
bool LoadSettings(char *filename, sParamRender &params, bool disableMessages = false);
void DefaultValues(sParamRender &params);
void IFSToMorph(double *IFSdouble, const sFractal &fractal);
void MorphToIFS(double *IFSdouble, sFractal &fractal);
void MakePaletteString(const sRGB *palette, char *paletteString);
void GetPaletteFromString(sRGB *palette, char *paletteString);
void KeepOtherSettings(sParamRender *params);
#endif /* SETTINGS_H_ */
